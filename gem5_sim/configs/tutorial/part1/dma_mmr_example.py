# DMA Controller MMR 使用示例
# 展示如何通过 MMR 寄存器控制 DMA，以及如何连接 dma_port 到 membus

import m5
from m5.objects import *

system = System()
voltage_domain = VoltageDomain()
system.clk_domain = SrcClockDomain(clock='1GHz', voltage_domain=voltage_domain)
system.mem_mode = "timing"
system.mem_ranges = [AddrRange("512MiB")]

# 创建 CPU
system.cpu = RiscvTimingSimpleCPU()
system.cpu.createInterruptController()

# 创建主总线
system.membus = SystemXBar()

# ============================================================================
# L1D SPM with DMA Controller
# ============================================================================
# L1D SPM 地址范围: 0x80000000 - 0x80010000 (64KB)
#   - 数据区域: 0x80000000 - 0x8000FF00 (64KB - 256B)
#   - MMR 区域:  0x8000FF00 - 0x80010000 (256B)
system.l1d_spm = L1DScratchpadMemory(
    clk_domain=system.clk_domain,
    range=AddrRange(start=0x80000000, size='64KiB'),
    mmr_size=256,  # MMR 占用 256 字节
    latency='2ns',
    bandwidth='32GiB/s'
)

# 连接 CPU 到 L1D SPM（通过 membus）
system.l1d_spm.port = system.membus.mem_side_ports

# ============================================================================
# DMA Port 连接到 membus
# 这样 DMA 可以访问系统中的其他内存区域
# ============================================================================
system.l1d_spm.dma_port = system.membus.cpu_side_ports

# ============================================================================
# 其他内存区域（DMA 可以访问）
# ============================================================================
# L2 SPM
system.l2_spm = SimpleMemory(
    clk_domain=system.clk_domain,
    range=AddrRange(start=0x90000000, size='1MiB'),
    latency='10ns',
    bandwidth='16GiB/s'
)
system.l2_spm.port = system.membus.mem_side_ports

# DDR
system.mem_ctrl = MemCtrl()
system.mem_ctrl.dram = DDR3_1600_8x8()
system.mem_ctrl.dram.range = system.mem_ranges[0]
system.mem_ctrl.port = system.membus.mem_side_ports

# 连接 CPU
system.cpu.icache_port = system.membus.cpu_side_ports
system.cpu.dcache_port = system.membus.cpu_side_ports
system.system_port = system.membus.cpu_side_ports

# ============================================================================
# Workload
# ============================================================================
binary = "tests/test-progs/hello/bin/riscv/linux/hello"
system.workload = SEWorkload.init_compatible(binary)
process = Process()
process.cmd = [binary]
system.cpu.workload = process
system.cpu.createThreads()

# ============================================================================
# Simulation
# ============================================================================
root = Root(full_system=False, system=system)
m5.instantiate()

print("=" * 70)
print("L1D SPM with DMA Controller - MMR Configuration")
print("=" * 70)
print(f"L1D SPM Range: {system.l1d_spm.range}")
print(f"  Data Region: 0x{system.l1d_spm.range.start:08x} - 0x{system.l1d_spm.range.end - 256:08x}")
print(f"  MMR Region:  0x{system.l1d_spm.range.end - 256:08x} - 0x{system.l1d_spm.range.end:08x}")
print()
print("MMR Register Map:")
print(f"  SRC_ADDR:  0x{system.l1d_spm.range.end - 256 + 0x00:08x} (64-bit)")
print(f"  DST_ADDR:  0x{system.l1d_spm.range.end - 256 + 0x08:08x} (64-bit)")
print(f"  SIZE:      0x{system.l1d_spm.range.end - 256 + 0x10:08x} (32-bit)")
print(f"  CMD:       0x{system.l1d_spm.range.end - 256 + 0x18:08x} (32-bit)")
print(f"  STATUS:    0x{system.l1d_spm.range.end - 256 + 0x20:08x} (32-bit)")
print()
print("DMA Port Connection:")
print("  L1D SPM dma_port -> membus -> L2 SPM / DDR")
print("=" * 70)

exit_event = m5.simulate()

print()
print("=" * 70)
print(f'Simulation finished @ tick {m5.curTick()}')
print(f'Exit reason: {exit_event.getCause()}')
print("=" * 70)
