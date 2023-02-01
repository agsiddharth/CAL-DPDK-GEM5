#include <unistd.h>

#include <cassert>
#include <iostream>
#include <vector>

#include <rte_config.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_mbuf.h>

static const char *kPacketMemPoolName = "dpdk_packet_mem_pool";

static constexpr size_t kDpdkArgcMax = 16;
static constexpr uint16_t kRingN = 1;
static constexpr uint16_t kRingDescN = 1024;

static constexpr size_t kMTUStandardFrames = 1500;
static constexpr size_t kMTUJumboFrames = 9000;

static constexpr size_t kLinkTimeOut_ms = 100;

static constexpr size_t kMaxBurst = 128;

void dispatch_thread(uint16_t port_id, uint16_t rx_ring_id, uint16_t tx_ring_id,
                     size_t total_packet_to_receive, uint16_t burst_size,
                     bool traffic_logs, size_t dummy_processing_cycles) {
  // Recv, process, and send packet.
  std::cout << "Running dispatch thread, polling for "
            << total_packet_to_receive << " packets with " << burst_size
            << " packet per burst...";

  rte_mbuf *packets[kMaxBurst];
  size_t pckt_cnt = 0;
  while (pckt_cnt < total_packet_to_receive) {
    // Poll.
    uint16_t received_pckt_cnt =
        rte_eth_rx_burst(port_id, rx_ring_id, packets, burst_size);
    if (received_pckt_cnt == 0)
      continue;

    // Get and print stats.
    if (traffic_logs) {
      constexpr size_t kTimeStampOffset = 8;
      std::cout << "A burst of " << received_pckt_cnt
                << " received, some data (timestamps) from them: \n";
      for (uint16_t i = 0; i < received_pckt_cnt; ++i) {
        uint64_t ts = *reinterpret_cast<uint64_t *>(
            rte_pktmbuf_mtod(packets[i], uint8_t *) + kTimeStampOffset);
        std::cout << ts << ", ";
      }
      std::cout << "\n";
    }

    // Simulate dummy processing.
    for (size_t delay = 0; delay < dummy_processing_cycles; ++delay) {
      asm volatile("");
    }

    // Send it back (L2 forward).
    uint16_t sent_pckt_cnt =
        rte_eth_tx_burst(port_id, tx_ring_id, packets, received_pckt_cnt);
    assert(sent_pckt_cnt == received_pckt_cnt);

    pckt_cnt += received_pckt_cnt;
  }

  std::cout << "All " << total_packet_to_receive
            << " packets are received and processed\n";
}

int main(int argc, char **argv) {
  // Parse input.
  // TODO: use smth like gflags
  // TODO: use glog for logging as well
  constexpr size_t kNumArgs = 4;
  size_t packets_to_process = 1000;
  uint16_t burst_size = 1;
  bool traffic_logs = false;
  size_t simulated_dummy_processing_cycles = 0;
  if (argc == kNumArgs + 1) {
    packets_to_process = std::atoi(argv[1]);
    burst_size = std::atoi(argv[2]);
    traffic_logs = std::atoi(argv[3]);
    simulated_dummy_processing_cycles = std::atoi(argv[4]);
  }
  if (burst_size > kMaxBurst) {
    std::cout << "Burst size can not be larger than " << kMaxBurst << "\n";
    return -1;
  }
  std::cout << "Invoking with packets_to_process= " << packets_to_process
            << ", burst_size= " << burst_size << ", with traffic logs "
            << (traffic_logs ? "enabled" : "disabled")
            << ", simulated_dummy_processing_cycles= "
            << simulated_dummy_processing_cycles << "\n";

  // Init EAL.
  int dargv_cnt = 0;
  char *dargv[kDpdkArgcMax];
  dargv[dargv_cnt++] = (char *)"-l";
  dargv[dargv_cnt++] = (char *)"0";
  dargv[dargv_cnt++] = (char *)"-n";
  dargv[dargv_cnt++] = (char *)"1";
  dargv[dargv_cnt++] = (char *)"--proc-type";
  dargv[dargv_cnt++] = (char *)"auto";

  int ret = rte_eal_init(dargv_cnt, dargv);
  if (ret < 0) {
    std::cout << "Cannot init EAL\n";
    return -1;
  }
  std::cout << "EAL is initialized!\n";

  // Look-up NICs.
  int p_num = rte_eth_dev_count_avail();
  if (p_num == 0) {
    std::cout << "No suitable NICs found; check driver binding and DPDK "
                 "linking options\n";
    return -1;
  }

  // Get PMD ports.
  std::vector<uint16_t> pmd_ports;
  for (uint16_t i = 0; i < RTE_MAX_ETHPORTS; ++i) {
    if (rte_eth_dev_is_valid_port(i))
      pmd_ports.push_back(i);
  }
  if (pmd_ports.size() == 0) {
    std::cout << "No valid ports found\n";
    return -1;
  }

  // Print MAC address for each valid port.
  std::cout << "Found " << p_num << " NIC ports: \n";
  for (auto &p_id : pmd_ports) {
    // MAC.
    std::cout << "    " << p_id << ", MAC: " << std::hex;
    rte_ether_addr mac_addr;
    rte_eth_macaddr_get(p_id, &mac_addr);
    for (int i = 0; i < RTE_ETHER_ADDR_LEN; ++i) {
      std::cout << static_cast<int>(mac_addr.addr_bytes[i])
                << ((i < RTE_ETHER_ADDR_LEN - 1) ? ":" : "");
    }
    std::cout << std::dec << "\n";
  }

  // Init a PMD port with one of the available and valid ports.
  uint16_t pmd_port_id = pmd_ports.front();
  std::cout << "Initializing port " << pmd_port_id << " with " << kRingN
            << " rings "
            << " and " << kRingDescN << " descriptors\n";
  // Fetch device info.
  rte_eth_dev_info dev_info;
  ret = rte_eth_dev_info_get(pmd_port_id, &dev_info);
  if (ret) {
    std::cout << "Failed to fetch device info\n";
    return -1;
  }
  // Make minimal Ethernet port configuration:
  //  - no checksum offload
  //  - no RSS
  //  - standard frames
  rte_eth_conf port_conf;
  memset(&port_conf, 0, sizeof(port_conf));
  port_conf.link_speeds = ETH_LINK_SPEED_AUTONEG;
  port_conf.rxmode.max_rx_pkt_len = kMTUStandardFrames;
  ret = rte_eth_dev_configure(pmd_port_id, kRingN, kRingN, &port_conf);
  if (ret) {
    std::cout << "Failed to configure the port\n";
    return -1;
  }
  ret = rte_eth_dev_set_mtu(pmd_port_id, kMTUStandardFrames);
  if (ret) {
    std::cout << "Failed to configure MTU size\n";
    return -1;
  }

  // Make packet pool.
  rte_mempool *mpool = rte_pktmbuf_pool_create(
      kPacketMemPoolName, kRingN * kRingDescN * 2, 0, 0,
      kMTUStandardFrames + RTE_PKTMBUF_HEADROOM, SOCKET_ID_ANY);
  if (mpool == nullptr) {
    std::cout << "Failed to create memory pool for packets.\n";
    return -1;
  }

  // Set-up RX/TX descs.
  uint16_t rx_ring_desc_N_actual = kRingDescN;
  uint16_t tx_ring_desc_N_actual = kRingDescN;
  ret = rte_eth_dev_adjust_nb_rx_tx_desc(pmd_port_id, &rx_ring_desc_N_actual,
                                         &tx_ring_desc_N_actual);
  if (ret) {
    std::cout << "Failed to adjust the number of RX descriptors.\n";
    return -1;
  }

  // Setup RX/TX rings (queues).
  for (size_t i = 0; i < kRingN; i++) {
    int ret = rte_eth_tx_queue_setup(pmd_port_id, i, tx_ring_desc_N_actual,
                                     static_cast<unsigned int>(SOCKET_ID_ANY),
                                     &dev_info.default_txconf);
    if (ret) {
      std::cout << "Failed to setup TX queues for ring " << i
                << "; exiting right now!\n";
      return -1;
    }

    ret = rte_eth_rx_queue_setup(pmd_port_id, i, rx_ring_desc_N_actual,
                                 static_cast<unsigned int>(SOCKET_ID_ANY),
                                 &dev_info.default_rxconf, mpool);
    if (ret) {
      std::cout << "Failed to setup RX queues for ring " << i
                << "; exiting right now!\n";
      return -1;
    }
  }

  // Start port.
  ret = rte_eth_dev_start(pmd_port_id);
  if (ret) {
    std::cout << "Failed to start port\n";
    return -1;
  }

  // Get link status.
  std::cout << "Port started, waiting for link to get up...\n";
  rte_eth_link link_status;
  memset(&link_status, 0, sizeof(link_status));
  size_t tout_cnt = 0;
  while (tout_cnt < kLinkTimeOut_ms &&
         link_status.link_status == ETH_LINK_DOWN) {
    memset(&link_status, 0, sizeof(link_status));
    rte_eth_link_get_nowait(pmd_port_id, &link_status);
    ++tout_cnt;
    usleep(1000);
  }
  if (link_status.link_status == ETH_LINK_UP)
    std::cout << "Link is UP and is ready to do packet I/O.\n";
  else {
    std::cout << "Link is DOWN.\n";
    return -1;
  }

  // Run dispatch thread to poll the port.
  dispatch_thread(pmd_port_id, 0, 0, packets_to_process, burst_size,
                  traffic_logs, simulated_dummy_processing_cycles);

  // Exit.
  std::cout << "Done, bye!\n";
  return 0;
}
