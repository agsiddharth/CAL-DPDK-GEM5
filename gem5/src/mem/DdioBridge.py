from m5.params import *
from m5.objects.ClockedObject import ClockedObject

class DdioBridge(ClockedObject):
    type = 'DdioBridge'
    cxx_class = 'gem5::DdioBridge'
    cxx_header = "mem/ddio_bridge.hh"
    cpuside = SlavePort('Slave port')
    mlcside = VectorMasterPort('Master port')
    llcside = MasterPort('Master port')
    memside = MasterPort('Master port')

    # SHIN. on-the-fly
    otfport = VectorSlavePort("For receiving on-the-fly data")
    
    # Test options
    do_not_pass_to_mlc = Param.Bool(False, "disable send to MLC. Like LLC DDIO ONLY")
    snoop_via_memside = Param.Bool(False, "Using memside for snooping")
    normal_DMA_mode = Param.Bool(False, "Off DDIO functions")

    window_size = Param.UInt32(200, "Window size for Forworad select (us)")
    threshhold_otf = Param.Float(0.75, "Thresh hold for bypass cache")
    threshhold_gradchange = Param.Float(0.25, "Thresh hold for bypass cache")
    threshhold_abs = Param.Float(0.03, "Send if under")

    # dic option
    ddio_option_app0 = Param.UInt32(0, "allow send to MLC")
    ddio_option_app1 = Param.UInt32(0, "allow send to MLC")
    ddio_option_app2 = Param.UInt32(0, "allow send to MLC")
    ddio_option_app3 = Param.UInt32(0, "allow send to MLC")

    dynamic_ddio = Param.UInt32(0, "Select dest dynamic")

    # MLC Sharing
    mlc_share = Param.UInt32(0, "Is mlc shared")

    # MLC-Prefetcher
    send_prefetch_hint = Param.Bool(True, "Send to Prefetcher")

    send_header_only = Param.Bool(False, "Send header only")

    #abstract = True