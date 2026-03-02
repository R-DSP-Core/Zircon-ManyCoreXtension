from m5.params import *
from m5.objects.Device import DmaDevice
from m5.proxy import *

class SimpleDMA(DmaDevice):
    type = "SimpleDMA"
    cxx_header = "R_DSP/SimpleDMA.hh"
    cxx_class = "gem5::SimpleDMA"

    pio_addr = Param.Addr("MMIO base address")
    pio_size = Param.Addr(0x100, "MMIO size")
    pio_delay = Param.Latency("100ns", "PIO delay")