from m5.params import *
from m5.objects.Device import DmaVirtDevice

class IDMA(DmaVirtDevice):
    type = 'IDMA'
    cxx_header = "mem/spm/idma.hh"
    cxx_class = "gem5::IDMA"
    abstract = False