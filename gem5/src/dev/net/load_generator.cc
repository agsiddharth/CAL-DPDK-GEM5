#include "dev/net/load_generator.hh"
#include <inttypes.h>

namespace gem5
{
    LoadGenerator::LoadGeneratorStats::LoadGeneratorStats(statistics::Group *parent)
        : statistics::Group(parent, "LoadGenerator"),
        ADD_STAT(sentPackets, statistics::units::Count::get(), "Number of Generated Packets"),
        ADD_STAT(recvPackets, statistics::units::Count::get(), "Number of Recieved Packets"),
        ADD_STAT(latency, statistics::units::Tick::get(), "Distribution of Latency in ticks")
        {
            sentPackets.precision(0);
            recvPackets.precision(0);
            latency.init(100);
        }


    LoadGenerator::LoadGenerator(const LoadGeneratorParams &p) : SimObject(p), packetSize(p.packet_size), packetRate(p.packet_rate), 
    startTick(p.start_tick), stopTick(p.stop_tick), checkLossInterval(100), incrementInterval(65536000/packetSize),// Whats a good value for this?
    lastRxCount(0), lastTxCount(0),
    sendPacketEvent([this]{sendPacket();}, name()), checkLossEvent([this]{checkLoss();}, name()), loadGeneratorStats(this)
    {
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
            schedule(sendPacketEvent, startTick);
    }

    Port & LoadGenerator::getPort(const std::string &if_name, PortID idx)
    {
        return *interface;
    }

    void LoadGenerator::sendPacket()
    {
        loadGeneratorStats.sentPackets++;
        lastTxCount++;

        EthPacketPtr txPacket = std::make_shared<EthPacketData>(packetSize);
        txPacket->length = packetSize;
        txPacket->data = new uint8_t[sizeof(uint64_t) + MACHeaderSize];
        
        uint64_t timeStamp = gem5::curTick();
        memcpy(&(txPacket->data[MACHeaderSize]), &timeStamp, sizeof(uint64_t));

        interface->sendPacket(txPacket);

        if (curTick() < stopTick)
        {
            if (((Tick)(loadGeneratorStats.sentPackets.value()) % checkLossInterval) == 0)
                // allow enough time for any in flight packets to be recieved
                schedule(checkLossEvent, curTick() + 1000000000);
            else
                schedule(sendPacketEvent, curTick() + frequency());
        }
    }

    void LoadGenerator::checkLoss()
    {
        if (lastTxCount - lastRxCount < 10)
        {
            packetRate = packetRate + incrementInterval;
            schedule(sendPacketEvent, curTick() + frequency());
            printf("Rate Incremented, now sending packets at %u \n", packetRate);
            printf("Rx %lu, Tx %lu \n", lastRxCount, lastTxCount);
        }
        else
        {
            if ((packetRate - incrementInterval) < packetRate)
                packetRate = packetRate - incrementInterval;
            // add extra delay to prevent previouse loss from affecting results
            schedule(sendPacketEvent, curTick() + frequency() + 1000000000);
            printf("Loss Detencted, now sending packets ever %u ticks \n", packetRate);
            printf("Rx %lu, Tx %lu \n", lastRxCount, lastTxCount);
        }
            lastTxCount = 0;
            lastRxCount = 0;
    }

    bool LoadGenerator::processRxPkt(EthPacketPtr pkt)
    {
        loadGeneratorStats.recvPackets++;
        lastRxCount++;

        uint64_t sendTick;
        memcpy(&sendTick, &(pkt->data[MACHeaderSize]), sizeof(uint64_t));
        
        float delta = float((gem5::curTick() - sendTick))/10.0e9;
        loadGeneratorStats.latency.sample(delta);
        return true;
    }
}