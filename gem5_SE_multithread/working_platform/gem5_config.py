import m5
from m5.objects import *
from m5.params import *
from gem5.components.cachehierarchies.classic.caches.l1dcache import L1DCache
from gem5.components.cachehierarchies.classic.caches.l1icache import L1ICache
from gem5.components.cachehierarchies.classic.caches.l2cache import L2Cache
from gem5.isas import ISA

# cpu core number
num_harts = 9

system = System()

system.clk_domain = SrcClockDomain()
system.derived_clk_domain = DerivedClockDomain(
  clk_domain = system.clk_domain,
  clk_divider = 1
)
system.clk_domain.clock = "1GHz"
system.clk_domain.voltage_domain = VoltageDomain()

# maximum physical range for 32-bit machine
system.mem_mode = "timing"
system.mem_ranges = [AddrRange("16GiB")]

# O3CPU cores with 2 ld/st pipeline
harts = [
  RiscvO3CPU(
    cacheStorePorts = 2,
    cacheLoadPorts  = 2
  ) for i in range(num_harts)
]

l1icache = [
  L1ICache(
    size = "32KiB",
    assoc= 2
  ) for i in range(num_harts)
]

l1dcache = [
  L1DCache(
    size = "32KiB",
    assoc = 2
  ) for i in range(num_harts)
]

l2crossbar = L2XBar()

l2cache = L2Cache(
  size = "2MiB",
  assoc = 4
)


system.cpu = harts 
system.l1dcache = l1dcache
system.l1icache = l1icache
system.l2crossbar = l2crossbar
system.l2cache = l2cache

for (i, hart, l1i, l1d) in zip([i for i in range(num_harts)], system.cpu, system.l1icache, system.l1dcache):
    hart.createInterruptController()
    hart.numThreads = 1
    hart.icache_port = l1i.cpu_side
    hart.dcache_port = l1d.cpu_side
    l1i.mem_side = system.l2crossbar.cpu_side_ports
    l1d.mem_side = system.l2crossbar.cpu_side_ports

system.l2crossbar.mem_side_ports = system.l2cache.cpu_side
system.l2crossbar.forward_latency = 1

system.membus = SystemXBar()
system.membus.width = 64
system.membus.frontend_latency = 1
system.membus.forward_latency = 0
system.membus.response_latency = 1 
system.membus.clk_domain = system.derived_clk_domain
system.membus.cpu_side_ports = system.l2cache.mem_side

system.mem_ctrl = MemCtrl()
system.mem_ctrl.dram = DDR4_2400_8x8()
system.mem_ctrl.dram.range = system.mem_ranges[0]
system.mem_ctrl.port = system.membus.mem_side_ports

system.system_port = system.membus.cpu_side_ports

# workload and SE simulation mode configs
thispath = os.path.dirname(os.path.realpath(__file__))
binary = os.path.join(
    "/home/lishunan/gemm/workload/gemm_pthread.out"
)

system.workload = RiscvSeWorkload.RiscvSEWorkload.init_compatible(binary)
system.redirect_paths = [RedirectPath(app_path="/lib", host_paths="/opt/riscv/sysroot/lib")]

system.num_work_ids = num_harts

#process = Process()
#process.cmd = [binary]

main_process = Process()
main_process.cmd = [binary]
main_process.executable = binary
main_process.pid = 100
main_process.pgid = 100
system.cpu[0].workload = main_process
system.cpu[0].createThreads()
system.cpu[0].ArchISA.riscv_type = "RV32"
system.cpu[0].ArchISA.enable_rvv = False
main_process.maxStackSize = "8MiB"

for i in range(1, num_harts):
  #process.pid = 100+i
  #process.pgid = 100
  #process.cmd = [binary]
  #process.executable = binary
  system.cpu[i].workload = main_process ### remember to allocate the workload to a list member
  system.cpu[i].createThreads()
  #system.cpu[i].system = system
  system.cpu[i].ArchISA.riscv_type = "RV32"  # set the isa to RV32
  #print(system.cpu[i].ArchISA)
  system.cpu[i].ArchISA.enable_rvv = False # disable the vector extension

root = Root(full_system=False, system=system)
m5.instantiate()

print(f"Beginning simulation! -----------------------------------------------\n")
exit_event = m5.simulate()
print(f"Exiting @ tick {m5.curTick()} because {exit_event.getCause()}")
print(f"\nSimulation at an end. -----------------------------------------------\n")
