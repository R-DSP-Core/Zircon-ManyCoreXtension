from gem5.components.boards.simple_board import SimpleBoard
from gem5.components.processors.simple_processor import SimpleProcessor
from gem5.components.cachehierarchies.classic.private_l1_shared_l2_cache_hierarchy import (
    PrivateL1SharedL2CacheHierarchy,
)
from gem5.components.memory.single_channel import SingleChannelDDR3_1600
from gem5.components.processors.cpu_types import CPUTypes
from gem5.isas import ISA
from gem5.resources.resource import CustomResource
from gem5.simulate.simulator import Simulator

import argparse

# 1. 解析参数
parser = argparse.ArgumentParser(description='A 4-core RISC-V system using gem5 Stdlib.')
parser.add_argument("binary", default="", nargs="?", type=str,
                    help="Path to the binary to execute.")
args = parser.parse_args()

# 设置默认 binary
if not args.binary:
    print("No binary specified. Using default: 'tests/test-progs/hello/bin/riscv/linux/hello'")
    args.binary = 'tests/test-progs/matmul/bin/main'

# 2. 定义组件
cache_hierarchy = PrivateL1SharedL2CacheHierarchy(
    l1d_size="64KiB",
    l1i_size="16KiB",
    l2_size="256KiB",
)

# Memory: 单通道 DDR3
memory = SingleChannelDDR3_1600("512MiB")

# Processor: 8核 RISC-V 乱序(O3)
processor = SimpleProcessor(
    cpu_type=CPUTypes.O3, 
    isa=ISA.RISCV, 
    num_cores=8
)

# 3. 组装主板 (Board)
board = SimpleBoard(
    clk_freq="1GHz",
    processor=processor,
    memory=memory,
    cache_hierarchy=cache_hierarchy,
)

# 4. 设置 Workload

binary_resource = CustomResource(args.binary)
board.set_se_binary_workload(binary_resource)

# 然后手动覆盖 CPU 的 Workload 分配来实现多线程共享
import m5.objects

# 创建一个共享的进程
process = m5.objects.Process(pid=100)
process.cmd = [args.binary]

# 将此进程分配给所有核心
# Stdlib 的 SimpleProcessor 封装了核心，我们通过 get_cores() 获取
print("cores:", processor.get_cores())
for core in processor.get_cores():
    core.get_simobject().workload = process


# 5. 运行仿真
simulator = Simulator(board=board)
print(f"开始 4 核 RISC-V Stdlib 仿真！运行程序: {args.binary}")
simulator.run()

print("仿真结束")

