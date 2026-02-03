# Copyright (c) 2025
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUGROUPS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

from m5.params import *
from m5.objects.SimpleMemory import SimpleMemory

class ScratchpadMemory(SimpleMemory):
    """
    L1 Instruction Scratchpad Memory with integrated DMA controller
    
    This SPM is READ-ONLY for CPU access (instruction fetch) but allows
    DMA transfers to load instructions. It provides:
    - A main port (inherited from SimpleMemory) for CPU core connections (Read-Only)
    - An integrated DMA controller for data transfers with next-level memory
    - Memory-Mapped Registers (MMR) for DMA control (Read-Write)
    - A DMA port for connecting to next-level memory (e.g., L2 SPM)
    
    The MMR region is located at the end of the SPM address range and
    includes registers for configuring DMA transfers:
    - SRC_ADDR: Source address for DMA read
    - DST_ADDR: Destination address for DMA write
    - SIZE: Transfer size in bytes
    - CMD: Command register (write 1 to start transfer)
    - STATUS: Status register (0=Idle, 1=Reading, 2=Writing)
    """

    type = 'ScratchpadMemory'
    cxx_header = "mem/spm/spm.hh"
    cxx_class = "gem5::memory::ScratchpadMemory"

    # Port for connecting to next-level memory via DMA
    dma_port = ResponsePort("DMA port for accessing next-level memory")

    # MMR (Memory-Mapped Register) configuration
    # MMR is accessed through the main port (inherited from SimpleMemory)
    # MMR region is located at the end of the SPM address range
    mmr_size = Param.Addr(0x100, "Size of the MMR region in bytes (default 256 bytes)")
