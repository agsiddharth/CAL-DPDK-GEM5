from m5.params import *
from m5.SimObject import SimObject
from m5.objects.Ethernet import EtherInt


class LoadGeneratorPcap(SimObject):
    type = 'LoadGeneratorPcap'
    cxx_header = "dev/net/load_generator_pcap.hh"
    cxx_class = 'gem5::LoadGeneratorPcap'

    interface = EtherInt("interface")
    loadgen_id = Param.Int(0, "For match NIC")
    start_tick =  Param.Tick(1,"Tick at whcih to start loadgenerator")
    stop_tick = Param.Tick(1, "Tick at which to stop loadgenerator")
    pcap_filename = Param.String("", "Filename of the pcap file")
    max_packetsize = Param.Int(1500, "To cut large packets")
    port_filter = Param.Int(11211, "Filter pcap traces by port")
    stack_mode = Param.String("KernelStack", "DPDKStack")
    replace_src_ip = Param.String("10.10.10.11", "Replace src IP to this value")
    replace_dest_ip = Param.String("10.10.10.10", "Replace dst IP to this value")
    replay_mode = Param.String("SimpleReplay", "SimpleReplay/ReplayAndAdjustThroughput/ConstThroughput")
    packet_rate = Param.Int(1, "To be used in ReplayAndAdjustThroughput/ConstThroughput")
    increment_interval = Param.Int(1, "To be used in ReplayAndAdjustThroughput")
