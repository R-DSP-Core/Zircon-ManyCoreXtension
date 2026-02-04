# SPM Test Configuration
# This configuration demonstrates:
# - L1D/L1I SPM with DMA controller
# - L2 SPM
# - SystemXBar (Crossbar) interconnect

import m5
from m5.objects import *

# Create system
system = System()

# Create Root immediately
root = Root(full_system=False, system=system)

# Clock and voltage
voltage_domain = VoltageDomain()
system.clk_domain = SrcClockDomain(clock='1GHz', voltage_domain=voltage_domain)
system.mem_mode = "timing"

# CPU
system.cpu = RiscvTimingSimpleCPU()
system.cpu.ArchISA.riscv_type = "RV32"
system.cpu.createInterruptController()

system.mem_ranges = [
    AddrRange(start=0x80000000, size='64kB'),  # 对应 L1i
    AddrRange(start=0x80010000, size='64kB'),  # 对应 L1d
    AddrRange(start=0x80020000, size='256kB'),  # 对应 L2
    AddrRange(start=0x80060000, size='4kB'),  # 对应 IDMA

]

system.xbar1 = IOXBar(
    frontend_latency=0,
    forward_latency=0,
    response_latency=0,
)

system.xbar2 = IOXBar(
    frontend_latency=0,
    forward_latency=0,
    response_latency=0,
)

# ============================================================================
# Memory Hierarchy
# ============================================================================

# L1I SPM (Instruction & Code) - 0x80000000
system.l1i_spm = ScratchpadMemory(
    clk_domain=system.clk_domain,
    range=AddrRange(start=0x80000000, size='64KiB'),
    latency='2ns',
    bandwidth='32GiB/s'
)

# L1D SPM (Data) - 0x80010000
system.l1d_spm = ScratchpadMemory(
    clk_domain=system.clk_domain,
    range=AddrRange(start=0x80010000, size='64KiB'),
    latency='2ns',
    bandwidth='32GiB/s'
)


# L2 SPM (Data) - 0x80020000
system.l2_spm = ScratchpadMemory(
    clk_domain=system.clk_domain,
    range=AddrRange(start=0x80020000, size='256KiB'),
    latency='2ns',
    bandwidth='32GiB/s'
)

system.idma=IDMA(
    # clk_domain=system.clk_domain,
    # pio_addr=AddrRange(start=0x80030000, size='4KiB'),
    # latency='1ns',
    # bandwidth='16GiB/s'
)

# ============================================================================
# Connections
# ============================================================================

# 1. Connect CPU to Bus
system.cpu.icache_port = system.xbar1.cpu_side_ports
system.cpu.dcache_port = system.xbar1.cpu_side_ports

system.xbar1.mem_side_ports = system.l1i_spm.port
system.xbar1.mem_side_ports = system.l1d_spm.port
system.xbar1.mem_side_ports = system.l2_spm.port
system.xbar1.mem_side_ports = system.idma.pio   # CPU 访问 IDMA 寄存器 (0x80060000) 必须接 pio

system.idma.dma = system.xbar2.cpu_side_ports

system.xbar2.mem_side_ports = system.l1i_spm.dma_port
system.xbar2.mem_side_ports = system.l1d_spm.dma_port
system.xbar2.mem_side_ports = system.l2_spm.dma_port


binary = "tests/test-progs/spm_test/bin/spm_test"
system.workload = SEWorkload.init_compatible(binary)

process = Process()
process.cmd = [binary]

system.cpu.workload = process
system.cpu.createThreads()

# ============================================================================
# Simulation
# ============================================================================
m5.instantiate()

# 恒等映射 (VA = PA): 虚拟地址 = 物理地址，无操作系统页表转换
process.map(0x80000000, 0x80000000, 0x10000)   # L1I SPM (Code)
process.map(0x80010000, 0x80010000, 0x10000)   # L1D SPM
process.map(0x80020000, 0x80020000, 0x40000, cacheable=True, clobber=True)  # L2 SPM
process.map(0x80060000, 0x80060000, 0x1000)    # IDMA 寄存器
print("=" * 70)
print("SPM Test Configuration (Bus-based)")
print("=" * 70)
print(f"L1I SPM: {system.l1i_spm.range}")
print(f"L1D SPM: {system.l1d_spm.range}")
print(f"L2 SPM:  {system.l2_spm.range}")
print("=" * 70)

exit_event = m5.simulate()

print()
print("=" * 70)
print(f'Simulation finished @ tick {m5.curTick()}')
print(f'Exit reason: {exit_event.getCause()}')
print("=" * 70)
