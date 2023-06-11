#ifndef __LOAD_GENERATOR_HH__
#define __LOAD_GENERATOR_HH__

#include <pcap/pcap.h>

#include "base/statistics.hh"
#include "dev/net/etherint.hh"
#include "params/LoadGeneratorPcap.hh"
#include "sim/eventq.hh"
#include "sim/sim_object.hh"

namespace gem5 {

class LoadGenPcapInt;

class LoadGeneratorPcap : public SimObject {
  // Configuration modes.
  enum class StackMode { Kernel, DPDK };
  enum class ReplayMode {
    SimpleReplay,
    ReplayAndAdjustThroughput,
    ConstThroughput
  };

 private:
  // Unique ID of this module to be used as a part of the "device"'s MAC
  // address.
  const uint8_t loadgenId;

  // General configs.
  LoadGenPcapInt *interface;
  const Tick startTick;
  const Tick stopTick;
  const size_t maxPcktSize;
  const uint16_t portFilter;
  const std::string srcIP, destIP;
  ReplayMode replayMode;
  uint32_t packetRate;
  Tick incrementInterval;

  // Stats for checking the loss.
  uint64_t lastRxCount;
  uint64_t lastTxCount;

  // Pcap related configuration.
  std::string pcapFilename;
  pcap_t *pcap_h;

  // Scheduling events.
  EventFunctionWrapper sendPacketEvent;
  EventFunctionWrapper checkLossEvent;

  struct LoadGeneratorPcapStats : public statistics::Group {
    LoadGeneratorPcapStats(statistics::Group *parent);
    statistics::Scalar sentPackets;
    statistics::Scalar recvPackets;
    statistics::Histogram latency;
  } loadGeneratorPcapStats;

  // Scheduling event callbacks.
  void sendPacket();
  void checkLoss();

  void endTest() const;

  // Incapsulate packet into Ethernet frame.
  void buildEthernetHeader(EthPacketPtr ethpacket) const;

  inline Tick pckt_freq() const { return 1e12 / packetRate; }

 public:
  LoadGeneratorPcap(const LoadGeneratorPcapParams &p);
  ~LoadGeneratorPcap();

  Port &getPort(const std::string &if_name, PortID idx);
  void startup();

  bool processRxPkt(EthPacketPtr pkt);
};

class LoadGenPcapInt : public EtherInt {
 private:
  LoadGeneratorPcap *dev;

 public:
  LoadGenPcapInt(const std::string &name, LoadGeneratorPcap *d)
      : EtherInt(name), dev(d) {}

  virtual bool recvPacket(EthPacketPtr pkt) { return dev->processRxPkt(pkt); }
  virtual void sendDone() { return; }
};

}  // namespace gem5

#endif  // __LOAD_GENERATOR_HH__
