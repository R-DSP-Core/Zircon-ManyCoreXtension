# Example configuration for L1Controller with three-way routing
# This demonstrates address-based routing to L1 SPM, IDMA, and DDR

import m5
from m5.objects import *

# Create a System object
system = System()

# Create voltage and clock domains
voltage_domain = VoltageDomain()
system.clk_domain = SrcClockDomain(clock='1GHz', voltage_domain=voltage_domain)

system.mem_mode = "timing"
system.mem_ranges = [AddrRange("512MiB")]

# Create a CPU
system.cpu = RiscvTimingSimpleCPU()

# RISC-V requires an interrupt controller per thread
system.cpu.createInterruptController()

# ============================================================================
# L1 Controller - Routes requests based on address ranges
# ============================================================================
system.l1_controller = L1Controller(
    clk_domain=system.clk_domain,
    l1_spm_range=AddrRange(start=0x80000000, size='64KiB'),
    idma_range=AddrRange(start=0x90000000, size='4KiB'),
    ddr_range=AddrRange(start=0x00000000, size='512MiB')
)

# Connect CPU to L1 Controller
system.cpu.icache_port = system.l1_controller.cpu_side_ports
system.cpu.dcache_port = system.l1_controller.cpu_side_ports

# ============================================================================
# Memory regions connected to L1 Controller
# ============================================================================

# 1. L1 SPM (Scratchpad Memory)
# Address range: 0x80000000 - 0x80010000 (64KB)
system.l1_spm = SimpleMemory(
    clk_domain=system.clk_domain,
    range=AddrRange(start=0x80000000, size='64KiB'),
    latency='2ns',
    bandwidth='32GiB/s'
)

# 2. IDMA (DMA Controller Registers)
# Address range: 0x90000000 - 0x90001000 (4KB)
system.idma = SimpleMemory(
    clk_domain=system.clk_domain,
    range=AddrRange(start=0x90000000, size='4KiB'),
    latency='1ns',
    bandwidth='16GiB/s'
)

# 3. DDR (Main Memory)
# Address range: 0x00000000 - 0x20000000 (512MB)
system.mem_ctrl = MemCtrl()
system.mem_ctrl.dram = DDR3_1600_8x8()
system.mem_ctrl.dram.range = system.mem_ranges[0]

# ============================================================================
# Connect memory regions to L1 Controller
# The controller will route requests based on address ranges
# ============================================================================
system.l1_spm.port = system.l1_controller.mem_side_ports
system.idma.port = system.l1_controller.mem_side_ports
system.mem_ctrl.port = system.l1_controller.mem_side_ports

# System port for functional access
system.system_port = system.l1_controller.cpu_side_ports

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
print("L1 Controller Configuration")
print("=" * 70)
print("Address routing:")
print(f"  L1 SPM:  {system.l1_spm.range} -> Port 0")
print(f"  IDMA:    {system.idma.range} -> Port 1")
print(f"  DDR:     {system.mem_ctrl.dram.range} -> Port 2")
print("=" * 70)
print("\nRouting logic:")
print("  - CPU requests to 0x80000000-0x8000FFFF -> L1 SPM")
print("  - CPU requests to 0x90000000-0x90000FFF -> IDMA")
print("  - CPU requests to 0x00000000-0x1FFFFFFF -> DDR")
print("=" * 70)
print("\nBeginning simulation!")
print()

exit_event = m5.simulate()

print()
print("=" * 70)
print(f'Simulation finished @ tick {m5.curTick()}')
print(f'Exit reason: {exit_event.getCause()}')
print("=" * 70)
