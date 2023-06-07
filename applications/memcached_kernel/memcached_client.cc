#include <gflags/gflags.h>
#include <memcached_client.h>
#include <zipfian_int_distribution.h>

#include <csignal>
#include <iostream>
#include <random>
#include <tuple>
#include <vector>

DEFINE_string(server_ip, "127.0.0.1", "IP address of the memcached server.");
DEFINE_uint32(server_port, 11211, "UDP port of the memcached server.");
DEFINE_bool(blocking, false,
            "Threading: false - non-blocking, true - blocking");
DEFINE_uint32(dataset_size, 2000, "Total size of the dataset");
DEFINE_string(dataset_key_size, "10-100-0.9",
              "Key size in the dataset, format: <min-max-skew>.");
DEFINE_string(dataset_val_size, "100-1000-0.9",
              "Value size in the dataset, format: <min-max-skew>.");
DEFINE_uint32(populate_workload_size, 1000,
              "Size of the sub-set of the dataset used for initial population "
              "of the memcached server.");
DEFINE_string(workload_config, "100-0.9-100",
              "The workload to execute, format: "
              "<number_of_queries-GET/(SET+GET)>-delay_cycles");

typedef std::vector<std::vector<uint8_t>> DSet;

static constexpr size_t kMaxValSize = 65536;

// To catch Ctl-C.
static volatile bool kCtlzArmed = false;
void signal_callback_handler(int signum) {
  (void)(signum);
  kCtlzArmed = true;
}

// ./memcached_client --server_ip=10.212.84.119 --blocking=false
// --dataset_size=5000 --dataset_key_size="10-100-0.9"
// --dataset_val_size="10-100-0.5" --populate_workload_size=2000
// --workload_config="10000-0.8-10000"
int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  // Register signal handler.
  signal(SIGINT, signal_callback_handler);

  // Init memcached client.
  std::cout << "Initializing a "
            << (FLAGS_blocking ? "blocking" : "non-blocking")
            << " memcached client"
            << "\n";
  MemcachedClient client(FLAGS_server_ip, FLAGS_server_port);
  client.Init();

  // Generate dataset.
  size_t ds_size = FLAGS_dataset_size;
  long unsigned int ksize_min, ksize_max, vsize_min, vsize_max;
  float ksize_skew, vsize_skew;
  sscanf(FLAGS_dataset_key_size.c_str(), "%lu-%lu-%f", &ksize_min, &ksize_max,
         &ksize_skew);
  sscanf(FLAGS_dataset_val_size.c_str(), "%lu-%lu-%f", &vsize_min, &vsize_max,
         &vsize_skew);
  if (vsize_max > kMaxValSize) {
    std::cout << "Value size of " << vsize_max << " is too big\n";
    return -1;
  }
  std::cout << "Generating dataset: #items: " << ds_size
            << ", key distribution: " << ksize_min << "|" << ksize_max << "|"
            << ksize_skew << ", value distribution: " << vsize_min << "|"
            << vsize_max << "|" << vsize_skew << "\n";

  DSet dset_keys;
  DSet dset_vals;
  std::default_random_engine generator;
  zipfian_int_distribution<size_t> k_distribution(ksize_min, ksize_max,
                                                  ksize_skew);
  zipfian_int_distribution<size_t> v_distribution(vsize_min, vsize_max,
                                                  vsize_skew);
  for (size_t i = 0; i < ds_size; ++i) {
    size_t key_len = k_distribution(generator);
    size_t val_len = v_distribution(generator);
    dset_keys.push_back(std::vector<uint8_t>());
    dset_vals.push_back(std::vector<uint8_t>());
    dset_keys.back().reserve(key_len);
    dset_vals.back().reserve(val_len);
    for (size_t j = 0; j < key_len; ++j) {
      dset_keys.back().push_back(std::rand() % 256);
    }
    for (size_t j = 0; j < val_len; ++j) {
      dset_vals.back().push_back(std::rand() % 256);
    }
  }
  std::cout << "Dataset generated.\n";

  // Populate memcached server with the dataset.
  size_t populate_ds_size = FLAGS_populate_workload_size;
  if (populate_ds_size > ds_size) {
    std::cout << "Population dataset is bigger than the main dataset.\n";
    return -1;
  } else {
    std::cout << "Populating memcached server with " << populate_ds_size
              << " first elements from the generated dataset.\n";
  }

  // We always populate in the blocking mode.
  client.setDispatchMode(MemcachedClient::kBlocking);
  size_t populate_errors = 0;
  for (size_t i = 0; i < populate_ds_size; ++i) {
    int res = client.set(i, 0, dset_keys[i].data(), dset_keys[i].size(),
                         dset_vals[i].data(), dset_vals[i].size());
    if (res != 0) ++populate_errors;
  }
  std::cout << "Server populated with " << populate_ds_size
            << " key-value pairs, "
            << " error count: " << populate_errors << "\n";
  client.zeroOutRecvStat();

  // Execute the load.
  std::cout << "Press <Ctrl-C> to execute the workload...\n";
  while (!kCtlzArmed) {
    sleep(1);
  }

  size_t wrkl_size;
  float wrkl_get_frac;
  size_t wrkl_delay;
  sscanf(FLAGS_workload_config.c_str(), "%lu-%f-%lu", &wrkl_size,
         &wrkl_get_frac, &wrkl_delay);
  size_t num_of_unique_sets = ds_size - populate_ds_size;
  std::cout << "Executing workload of #queries: " << wrkl_size
            << ", GET/SET= " << wrkl_get_frac
            << ", unique SET keys: " << num_of_unique_sets
            << ", blocking?: " << FLAGS_blocking << "\n";

  size_t get_errors = 0;
  size_t get_data_errors = 0;
  size_t set_cnt = 0;
  size_t set_errors = 0;
  uint8_t val_ret[kMaxValSize];
  uint32_t val_ret_length = 0;

  // Set dispatch semantics based on the configuration.
  client.setDispatchMode(FLAGS_blocking ? MemcachedClient::kBlocking
                                        : MemcachedClient::kNonBlocking);

  struct timespec wrkl_start, wrkl_end;
  clock_gettime(CLOCK_MONOTONIC, &wrkl_start);
  for (size_t i = 0; i < wrkl_size; ++i) {
    // Blocking delay to control the rps rate.
    for (size_t delay = 0; delay < wrkl_delay; ++delay) {
      asm("");
    }

    float get_set = rand() / (float)RAND_MAX;
    if (get_set < wrkl_get_frac) {
      // Execute GET.
      // Always hit in the cache, i.e. use a populated key.
      size_t random_key_idx = static_cast<size_t>(rand()) % populate_ds_size;
      auto& key = dset_keys[random_key_idx];
      int res =
          client.get(i, 0, key.data(), key.size(), val_ret, &val_ret_length);
      if (res == 0) {
        if (FLAGS_blocking) {
          // Check result.
          auto val = dset_vals[random_key_idx];
          if (val.size() != val_ret_length ||
              std::memcmp(val.data(), val_ret, val.size()) != 0) {
            ++get_data_errors;
          }
        }
      } else {
        ++get_errors;
      }
    } else {
      // Execute SET.
      // Always miss in the cache, i.e. use an unpopulated key.
      size_t key_idx = populate_ds_size + (set_cnt % num_of_unique_sets);
      int res =
          client.set(i, 0, dset_keys[key_idx].data(), dset_keys[key_idx].size(),
                     dset_vals[key_idx].data(), dset_vals[key_idx].size());
      if (res != 0) ++set_errors;
      ++set_cnt;
    }
  }
  clock_gettime(CLOCK_MONOTONIC, &wrkl_end);
  static constexpr long int kBillion = 1000000000L;
  long int wrkl_diff = kBillion * (wrkl_end.tv_sec - wrkl_start.tv_sec) +
                       wrkl_end.tv_nsec - wrkl_start.tv_nsec;
  double wrkl_ns = wrkl_diff / (double)wrkl_size;
  double wrkl_avg_thr = kBillion * (1 / wrkl_ns);  // qps

  std::cout << "Workload executed, some statistics: \n";
  std::cout << "   * total requests sent: " << wrkl_size << "\n";
  std::cout << "   * total requests received: "
            << (FLAGS_blocking ? wrkl_size : client.dumpRecvStat()) << "\n";
  std::cout << "   * average sending throughput: " << wrkl_avg_thr << " qps\n";
  std::cout << "   * GET errors: " << get_errors << "\n";
  std::cout << "   * GET data errors: " << get_data_errors << "\n";
  std::cout << "   * SET errors: " << set_errors << "\n";

  return 0;
}
