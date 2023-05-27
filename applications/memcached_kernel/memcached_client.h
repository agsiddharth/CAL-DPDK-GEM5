#ifndef _MEMCACHED_CLIENT_H_
#define _MEMCACHED_CLIENT_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

static constexpr size_t kClSize = 64;
static constexpr size_t kMaxPacketSize = 65536;

class MemcachedClient {
 public:
  enum Status {
    kOK = 0x0,
    kKeyNotFound = 0x1,
    kKeyExists = 0x2,
    kValueTooLarge = 0x3,
    kInvalidArgument = 0x4,
    kItemNotStored = 0x5,
    kNotAValue = 0x6,
    kUnknownComand = 0x81,
    kOutOfMemory = 0x82,
    kOtherError = 0xff
  };

  MemcachedClient(const std::string &server_hostname, uint16_t port)
      : serverHostname(server_hostname),
        port(port),
        sock(-1),
        tx_buff(nullptr),
        rx_buff(nullptr) {}

  ~MemcachedClient() {
    if (tx_buff != nullptr) std::free(tx_buff);
    if (rx_buff != nullptr) std::free(rx_buff);
    if (sock != -1) close(sock);
  }

  int Init() {
    // Init networking.
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    if (inet_pton(AF_INET, serverHostname.c_str(), &(serverAddress.sin_addr)) <=
        0) {
      std::cerr << "Invalid address or address not supported." << std::endl;
      return -1;
    }

    std::cout << serverHostname.c_str() << "\n";

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
      std::cerr << "Failed to create socket." << std::endl;
      return -1;
    }

    // Init buffers.
    tx_buff =
        static_cast<uint8_t *>(std::aligned_alloc(kClSize, kMaxPacketSize));
    assert(tx_buff != nullptr);

    rx_buff =
        static_cast<uint8_t *>(std::aligned_alloc(kClSize, kMaxPacketSize));
    assert(rx_buff != nullptr);

    return 0;
  }

  int set(uint16_t request_id, uint16_t sequence_n, const uint8_t *key,
          uint16_t key_len, const uint8_t *val, uint32_t val_len,
          bool wait_for_response = true) {
    uint32_t len = 0;
    int res =
        form_set(request_id, sequence_n, key, key_len, val, val_len, &len);
    if (res != 0) return res;

    res = send(len);
    if (res != 0) return res;

    if (wait_for_response) {
      uint16_t req_id_rcv;
      uint16_t seq_n_recv;
      std::cout << "req_id_rcv= " << req_id_rcv
                << ", seq_n_recv= " << seq_n_recv << "\n";
      recv();
      int ret = parse_set_response(&req_id_rcv, &seq_n_recv);
      if (ret != kOK) // || req_id_rcv != request_id)
        return -1;
      else
        return 0;
    }

    return 0;
  }

  int get(uint16_t request_id, uint16_t sequence_n, const uint8_t *key,
          uint16_t key_len, uint8_t *val_out, uint32_t *val_out_length,
          bool wait_for_response = true) {
    uint32_t len = 0;
    int res = form_get(request_id, sequence_n, key, key_len, &len);
    if (res != 0) return res;

    res = send(len);
    if (res != 0) return res;

    if (wait_for_response) {
      assert(val_out != nullptr);
      assert(val_out_length != nullptr);

      uint16_t req_id_rcv;
      uint16_t seq_n_recv;
      recv();
      int ret =
          parse_get_response(&req_id_rcv, &seq_n_recv, val_out, val_out_length);
      if (ret != kOK) // || req_id_rcv != request_id)
        return -1;
      else
        return 0;
    }

    return 0;
  }

 private:
  // Net things.
  std::string serverHostname;
  uint16_t port;
  sockaddr_in serverAddress;
  int sock;

  // Buffers.
  uint8_t *tx_buff;
  uint8_t *rx_buff;

  // Memcached has it's own extra header for the UDP protocol; it's not really
  // documented, but can be found here: memcached.c:build_udp_header().
  struct MemcacheUdpHeader {
    uint8_t request_id[2];
    uint8_t udp_sequence[2];
    uint8_t udp_total[2];
    uint8_t RESERVED[2];
  } __attribute__((packed));
  static_assert(sizeof(MemcacheUdpHeader) == 8);

  struct ReqHdr {
    uint8_t magic;
    uint8_t opcode;
    uint8_t key_length[2];
    uint8_t extra_length;
    uint8_t data_type;
    uint8_t RESERVED[2];
    uint8_t total_body_length[4];
    uint8_t opaque[4];
    uint8_t CAS[8];
  } __attribute__((packed));
  static_assert(sizeof(ReqHdr) == 24);

  struct RespHdr {
    uint8_t magic;
    uint8_t opcode;
    uint8_t key_length[2];
    uint8_t extra_length;
    uint8_t data_type;
    uint8_t status[2];
    uint8_t total_body_length[4];
    uint8_t opaque[4];
    uint8_t CAS[8];
  } __attribute__((packed));
  static_assert(sizeof(RespHdr) == 24);

  int form_set(uint16_t request_id, uint16_t sequence_n, const uint8_t *key,
               uint16_t key_len, const uint8_t *val, uint32_t val_len,
               uint32_t *pckt_length) {
    // Form memcached UDP header.
    MemcacheUdpHeader hdr = {
        .request_id = {static_cast<uint8_t>((request_id >> 8) & 0xff),
                       static_cast<uint8_t>(request_id & 0xff)},
        .udp_sequence = {static_cast<uint8_t>((sequence_n >> 8) & 0xff),
                         static_cast<uint8_t>(sequence_n & 0xff)},
        .udp_total = {0, 1},
        .RESERVED = {0, 0}};
    std::memcpy(tx_buff, &hdr, sizeof(MemcacheUdpHeader));

    // Form packet.
    constexpr uint8_t kExtraSize = 8;
    uint32_t total_payld_length = kExtraSize + key_len + val_len;
    uint32_t total_length =
        sizeof(MemcacheUdpHeader) + sizeof(ReqHdr) + total_payld_length;
    if (total_length > kMaxPacketSize) {
      std::cerr << "Packet size of " << total_length << " is too large\n";
      return -1;
    }
    ReqHdr req_hdr = {
        .magic = 0x80,
        .opcode = 0x01,  // set
        .key_length = {static_cast<uint8_t>((key_len >> 8) & 0xff),
                       static_cast<uint8_t>(key_len & 0xff)},
        .extra_length = kExtraSize,
        .data_type = 0x00,
        .RESERVED = 0x00,
        .total_body_length =
            {static_cast<uint8_t>((total_payld_length >> 24) & 0xff),
             static_cast<uint8_t>((total_payld_length >> 16) & 0xff),
             static_cast<uint8_t>((total_payld_length >> 8) & 0xff),
             static_cast<uint8_t>(total_payld_length & 0xff)},
        .opaque = 0x00,
        .CAS = 0x00};
    // Fill packet: unlimited storage time.
    uint32_t extra[2] = {0x00, 0x00};
    std::memcpy(tx_buff + sizeof(MemcacheUdpHeader), &req_hdr, sizeof(ReqHdr));
    std::memcpy(tx_buff + sizeof(MemcacheUdpHeader) + sizeof(ReqHdr), extra,
                kExtraSize);
    std::memcpy(
        tx_buff + sizeof(MemcacheUdpHeader) + sizeof(ReqHdr) + kExtraSize, key,
        key_len);
    std::memcpy(tx_buff + sizeof(MemcacheUdpHeader) + sizeof(ReqHdr) +
                    kExtraSize + key_len,
                val, val_len);

    *pckt_length = total_length;
    return 0;
  }

  int form_get(uint16_t request_id, uint16_t sequence_n, const uint8_t *key,
               uint16_t key_len, uint32_t *pckt_length) {
    // Form memcached UDP header.
    MemcacheUdpHeader hdr = {
        .request_id = {static_cast<uint8_t>((request_id >> 8) & 0xff),
                       static_cast<uint8_t>(request_id & 0xff)},
        .udp_sequence = {static_cast<uint8_t>((sequence_n >> 8) & 0xff),
                         static_cast<uint8_t>(sequence_n & 0xff)},
        .udp_total = {0, 1},
        .RESERVED = {0, 0}};
    std::memcpy(tx_buff, &hdr, sizeof(MemcacheUdpHeader));

    // Form packet.
    uint32_t total_payld_length = key_len;
    uint32_t total_length =
        sizeof(MemcacheUdpHeader) + sizeof(ReqHdr) + total_payld_length;
    if (total_length > kMaxPacketSize) {
      std::cerr << "Packet size of " << total_length << " is too large\n";
      return -1;
    }
    ReqHdr req_hdr = {
        .magic = 0x80,
        .opcode = 0x00,  // get
        .key_length = {static_cast<uint8_t>((key_len >> 8) & 0xff),
                       static_cast<uint8_t>(key_len & 0xff)},
        .extra_length = 0x00,
        .data_type = 0x00,
        .RESERVED = 0x00,
        .total_body_length =
            {static_cast<uint8_t>((total_payld_length >> 24) & 0xff),
             static_cast<uint8_t>((total_payld_length >> 16) & 0xff),
             static_cast<uint8_t>((total_payld_length >> 8) & 0xff),
             static_cast<uint8_t>(total_payld_length & 0xff)},
        .opaque = 0x00,
        .CAS = 0x00};
    // Fill packet.
    std::memcpy(tx_buff + sizeof(MemcacheUdpHeader), &req_hdr, sizeof(ReqHdr));
    std::memcpy(tx_buff + sizeof(MemcacheUdpHeader) + sizeof(ReqHdr), key,
                key_len);

    *pckt_length = total_length;
    return 0;
  }

  Status parse_set_response(uint16_t *request_id, uint16_t *sequence_n) const {
    const MemcacheUdpHeader *udp_hdr =
        reinterpret_cast<MemcacheUdpHeader *>(rx_buff);
    *request_id = (udp_hdr->request_id[1] << 8) | (udp_hdr->request_id[0]);
    *sequence_n = (udp_hdr->udp_sequence[1] << 8) | (udp_hdr->udp_sequence[0]);

    const RespHdr *rsp_hdr =
        reinterpret_cast<RespHdr *>(rx_buff + sizeof(MemcacheUdpHeader));
    if (rsp_hdr->magic != 0x81) {
      std::cerr << "Wrong response received: " << (int)(rsp_hdr->magic) << "\n";
      return kOtherError;
    }

    Status status =
        static_cast<Status>(rsp_hdr->status[1] | (rsp_hdr->status[0] << 8));
    return status;
  }

  Status parse_get_response(uint16_t *request_id, uint16_t *sequence_n,
                            uint8_t *val, uint32_t *val_len) const {
    const MemcacheUdpHeader *udp_hdr =
        reinterpret_cast<MemcacheUdpHeader *>(rx_buff);
    *request_id = (udp_hdr->request_id[1] << 8) | (udp_hdr->request_id[0]);
    *sequence_n = (udp_hdr->udp_sequence[1] << 8) | (udp_hdr->udp_sequence[0]);

    const RespHdr *rsp_hdr =
        reinterpret_cast<RespHdr *>(rx_buff + sizeof(MemcacheUdpHeader));
    if (rsp_hdr->magic != 0x81) {
      std::cerr << "Wrong response received!\n";
      return kOtherError;
    }

    Status status =
        static_cast<Status>(rsp_hdr->status[1] | (rsp_hdr->status[0] << 8));
    if (status == kOK) {
      uint8_t extra_len_ = rsp_hdr->extra_length;
      uint16_t key_len_ =
          rsp_hdr->key_length[1] | (rsp_hdr->key_length[0] << 8);
      uint32_t total_body_len = rsp_hdr->total_body_length[3] |
                                (rsp_hdr->total_body_length[2] << 8) |
                                (rsp_hdr->total_body_length[1] << 16) |
                                (rsp_hdr->total_body_length[0] << 24);
      uint32_t val_len_ = total_body_len - extra_len_ - key_len_;
      std::memcpy(val,
                  rx_buff + sizeof(MemcacheUdpHeader) + sizeof(RespHdr) +
                      extra_len_ + key_len_,
                  val_len_);

      *val_len = val_len_;
    }

    return status;
  }

  int send(size_t length) {
    ssize_t bytesSent =
        sendto(sock, tx_buff, length, MSG_CONFIRM,
               (const struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (bytesSent != length) {
      std::cerr << "Failed to send data to the server." << std::endl;
      close(sock);
      return -1;
    }

    return 0;
  }

  void recv() {
    sockaddr_in serverAddress_rvc;
    socklen_t len;
    recvfrom(sock, rx_buff, kMaxPacketSize, 0,
             (struct sockaddr *)&serverAddress_rvc, &len);
    // for (int i = 0; i < bytesRecv; ++i) std::cout << (char)(rx_buff[i]) << "
    // ";
  }
};

#endif
