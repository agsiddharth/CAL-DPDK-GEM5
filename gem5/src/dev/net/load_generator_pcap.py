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
    pcap_filename = Param.String("")
    max_packetsize = Param.Int(1500, "To cut large packets")
    port_filter = Param.Int(11211, "Filter pcap traces by port")
    stack_mode = Param.String("KernelStack", "DPDKStack")
    reply_mode = Param.String("SimpleReply", "ReplyAndAdjustThroughput")
    replace_dest_ip = Param.String("127.0.0.1")
