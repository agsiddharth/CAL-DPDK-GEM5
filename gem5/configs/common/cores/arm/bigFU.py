from m5.objects import *
from m5.objects.FuncUnit import *
from m5.objects.FuncUnitConfig import *




class bigFU(FUPool):
    FUList = [ IntALU(), IntMultDiv(), FP_ALU(), FP_MultDiv(), ReadPort(),
               SIMD_Unit(), PredALU(), WritePort(), RdWrPort(), WritePort(), RdWrPort(), IprPort() ]

class bigLSU(DerivO3CPU):
    numROBEntries = 384
    LQEntries = 64
    SQEntries = 64
    fuPool = bigFU()