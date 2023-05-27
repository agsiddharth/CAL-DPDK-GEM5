#include <memcached_client.h>

#include <iostream>

int main() {
  MemcachedClient client("10.212.84.119", 11211);
  client.Init();

  for (int i=0; i<1000; ++i) {

  int res = client.set(1, 0, reinterpret_cast<const uint8_t*>("rrrrr"), 5,
                       reinterpret_cast<const uint8_t*>("tktkt"), 5);
  if (res == 0)
    std::cout << "set succeded!\n";
  else
    std::cout << "set failed!\n"
              << "\n";

  uint8_t val[65535];
  uint32_t val_len;
  int res_1 = client.get(2, 0, reinterpret_cast<const uint8_t*>("rrrrr"), 5,
                         val, &val_len);
  if (res_1 == 0) {
    std::cout << "get succeded!\n";
    std::cout << "returned: " << val << "\n";
  } else
    std::cout << "get failed!\n"
              << "\n";

  }

  return 0;
}
