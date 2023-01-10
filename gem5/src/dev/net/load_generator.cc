#include "dev/net/load_generator.hh"
#include <inttypes.h>
#include "sim/sim_exit.hh"
#include <netinet/in.h>
#include "base/trace.hh"
#include "debug/LoadgenDebug.hh"
#include "debug/LoadgenLatency.hh"


namespace gem5
{
    LoadGenerator::LoadGeneratorStats::LoadGeneratorStats(statistics::Group *parent)
        : statistics::Group(parent, "LoadGenerator"),
        ADD_STAT(sentPackets, statistics::units::Count::get(), "Number of Generated Packets"),
        ADD_STAT(recvPackets, statistics::units::Count::get(), "Number of Recieved Packets"),
        ADD_STAT(latency, statistics::units::Second::get(), "Distribution of Latency in ms")
        {
            sentPackets.precision(0);
            recvPackets.precision(0);
            latency.init(100);
        }


    LoadGenerator::LoadGenerator(const LoadGeneratorParams &p) : SimObject(p), loadgenId(p.loadgen_id), packetSize(p.packet_size), packetRate(p.packet_rate), 
    startTick(p.start_tick), stopTick(p.stop_tick), checkLossInterval(1000), incrementInterval(5e+8/(packetSize*8)),// Whats a good value for this?
    burstWidth(p.burst_width), burstGap(p.burst_gap), burstStartTick(0),
    lastRxCount(0), lastTxCount(0),
    sendPacketEvent([this]{sendPacket();}, name()), checkLossEvent([this]{checkLoss();}, name()), loadGeneratorStats(this)
    {
        if (p.mode == "Static")
            loadgenMode = Mode::Static;
        else if (p.mode == "Increment")
            loadgenMode = Mode::Increment;
        else if (p.mode == "Burst")
            loadgenMode = Mode::Burst;

        LoadGenerator::interface = new LoadGenInt("interface", this);
    }

    Tick LoadGenerator::frequency()
    {
        return (1e12/packetRate);
    }

    void LoadGenerator::startup()
    {
        if (curTick() > stopTick) return;
        
        if (curTick() > startTick)
            schedule(sendPacketEvent, curTick() + 1);
        else
            schedule(sendPacketEvent, startTick + 1);
    }

    Port & LoadGenerator::getPort(const std::string &if_name, PortID idx)
    {
        return *interface;
    }

    void LoadGenerator::buildPacket(EthPacketPtr ethpacket)
    {
        // Build Packet header
        // DSTMAC 6 | SRCMAC 6 | LENGTH 2 | DATA
        uint8_t dst_mac[6] = {0x00, 0x90, 0x00, 0x00, 0x00, 0x01 + loadgenId}; // Use paired NIC's MAC
        uint8_t src_mac[6] = {0x00, 0x80, 0x00, 0x00, 0x00, 0x01 + loadgenId};

        uint16_t size = ethpacket->length;

        if(1 != htons(1)) size = htons(size);

        uint8_t head[MACHeaderSize];
        memcpy(head, dst_mac, 6);
        memcpy(head + 6, src_mac, 6);
        memcpy(head + 12, &size, 2);
        memcpy(ethpacket->data, head, MACHeaderSize);
        uint64_t timeStamp = gem5::curTick();
        memcpy(&(ethpacket->data[8]), &timeStamp, sizeof(uint64_t));
    }

    void LoadGenerator::sendPacket()
    {
        loadGeneratorStats.sentPackets++;
        lastTxCount++;

        EthPacketPtr txPacket = std::make_shared<EthPacketData>(packetSize);
        txPacket->length = packetSize;
        // only allocate enough mem to store timestamp
        txPacket->data = new uint8_t[2*sizeof(uint64_t)];
        buildPacket(txPacket);
        interface->sendPacket(txPacket);
        if (curTick() < stopTick)
        {
            if (loadgenMode == Mode::Increment)
            {
                if (lastTxCount == checkLossInterval)
                    // allow enough time for any in flight packets to be recieved
                    schedule(checkLossEvent, curTick() + 100000000);
                else
                    schedule(sendPacketEvent, curTick() + frequency());
            } else if (loadgenMode == Mode::Static)
            {
                schedule(sendPacketEvent, curTick() + frequency());
            } else if (loadgenMode == Mode::Burst)
            {
                if (curTick() - burstStartTick > burstWidth)
                    {
                        burstStartTick = curTick() + burstGap;
                        DPRINTF(LoadgenDebug, "Burst Ended, next Burst Starts at %lu \n", burstStartTick);
                        schedule(sendPacketEvent, burstStartTick);
                    }
                else 
                    schedule(sendPacketEvent, curTick() + frequency());
            }
        }
    }

    void LoadGenerator::checkLoss()
    {
        if (lastTxCount - lastRxCount < 10)
        {
            packetRate = packetRate + incrementInterval;
            schedule(sendPacketEvent, curTick() + frequency());
            DPRINTF(LoadgenDebug, "Rate Incremented, now sending packets at %u \n", packetRate);
            DPRINTF(LoadgenDebug, "Rx %lu, Tx %lu \n", lastRxCount, lastTxCount);
        }
        else
        {
            if ((packetRate - incrementInterval) < packetRate)
                packetRate = packetRate - incrementInterval;
            
            // add extra delay to prevent previouse loss from affecting results
            schedule(sendPacketEvent, curTick() + frequency() + 100000000);
            DPRINTF(LoadgenDebug, "Loss Detected, now sending packets at %u \n", packetRate);
            DPRINTF(LoadgenDebug, "Rx %lu, Tx %lu \n", lastRxCount, lastTxCount);
        }
            lastTxCount = 0;
            lastRxCount = 0;
    }

    void LoadGenerator::endTest()
    {
        exitSimLoop("m5_exit by loadgen End Simulator.", 0, curTick(), 0, true);
    }

    bool LoadGenerator::processRxPkt(EthPacketPtr pkt)
    {
        loadGeneratorStats.recvPackets++;
        lastRxCount++;

        uint64_t sendTick;
        memcpy(&sendTick, &(pkt->data[8]), sizeof(uint64_t));
        
        float delta = float((gem5::curTick() - sendTick))/10.0e3;
        loadGeneratorStats.latency.sample(delta);
        DPRINTF(LoadgenLatency, "Latency %f \n", delta);
        return true;
    }
}