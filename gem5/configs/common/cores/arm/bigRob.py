from m5.objects import *


class bigROB(DerivO3CPU):
    numROBEntries = 384
    LQEntries = 64
    SQEntries = 64