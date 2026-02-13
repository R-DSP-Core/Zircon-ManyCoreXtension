### 1 系统配置

1) CPU内核和共享二级缓存位于源时钟域，系统总线位于二分频时钟域

```python
system.derived_clk_domain = DerivedClockDomain(
  clk_domain = system.clk_domain,
  clk_divider = 1
)

system.membus.clk_domain = system.derived_clk_domain
```

2) hart数目：9个，一个用于进入进程，创建剩下八个子线程，main执行完之后退出进程；计算任务由剩下的八个核心完成
3) CPU类型：RiscvO3CPU，配置DefaultFUPool为双RdWrPort()，等价为两条ld/st流水线

```python
#~/gem5/src/cpu/o3/FUPool.py
class DefaultFUPool(FUPool):
FUList = [
        IntALU(),
        IntMultDiv(),
        FP_ALU(),
        FP_MultDiv(),
        SIMD_Unit(),
        Matrix_Unit(),
        System_Unit(),
        PredALU(),
        RdWrPort(), # ld/st流水线1
        RdWrPort()  # ld/st流水线2
]
```

4) 为了适应两条ld/st流水线，配置乱序CPU的cacheStorePorts和cacheLoadPorts均为2 (源码默认200)，这两个python参数会被分别传递给底层C类的cacheStorePorts和cacheLoadPorts
5) 系统组件通信互连设置：所有CPU内核和L2的互连采用protocol/implementation-irrelevant generic L2Xbar，类定义是``class X2bar(CoherentXbar)``；系统总线使用一致性SystemXbar，位于500MHz时钟域，为了协调和L2Xbar的吞吐量，其数据信道位宽改为64B，因为当前没有其他主机设备挂载在Membus上，forward_latency设为0，其他latency设置为1

```python
# 默认的L2XBar类定义: ~/gem5/src/mem/L2Xbar.py
class L2XBar(CoherentXBar):
    width = 32 # 单位是字节

    # 对应于AMBA AXI4/ACE协议，
    # > frontend_latency是address channel transaction，典型时间是一个周期;
    # > forward_latency在一致性事务 (coherence transaction)，只需要完成监
    #   听事务映射和snoop history cache的查表，该过程可以在address channel 
    #   transaction阶段并行处理，或者单另查表，0~2个周期比较典型 (1~2周期适用
    #   于事务转译流水化场景)；
    # > response_latency对应write data transaction + write response 
    #   transaction 和 read data/response transaction，对于32位机，典型值是
    #   1~3个周期，视实现而定，比如如果是critical word first，调用WRAP burst，
    #   在索取未命中缓存行的时候只需要一个周期，得到需要的字节即可以开始工作；如果
    #   实现的是缓存行的64B全部取回来之后重新命中，需要2个周期，这里使用第一种实现
    # > snoop_respopnse_latency，监听完成，一个周期可以完成
    frontend_latency = 1
    forward_latency = 0
    response_latency = 1
    snoop_response_latency = 1

    # ACE协议中支持optional external snoop filter，用以降低密集的一致性事务导致的
    # 监听洪泛现象，实验中保持默认设置，look_latency=0
    snoop_filter = SnoopFilter(lookup_latency=0)

    point_of_unification = True

# 实验中l2crossbar的设置
system.l2crossbar.forward_latency = 1

# 实验中的membus设置
system.membus.width = 64
system.membus.frontend_latency = 1
system.membus.forward_latency = 0
system.membus.respond_latency = 1
```

6) gem5的cacheline size默认64B，强制一二级缓存有相同的缓存行大小，一级缓存不论I/D，都使用2组联，二级缓存使用4组联；l1i/d cache全是采用当今常见32KB，l2 cache容量根据实验需求变动，上限为8MB，这是多核DSP常见的二级缓存总容量
7) 内存配置采用16GB DDR4 8X8，采用gem5默认配置
8) pthread矩阵乘法采用左乘矩阵行分块，右乘矩阵存储采用转置，缓解stride访存带来的带宽浪费；矩阵规模设置为128×128大小
9) 多线程运行时间计算：在Linux下使用gem5 debug flags=SyscallAll重定向操作导出的run.log中会记载所有系统调用时间，因为没有设置所有的线程在创建之后等待Barrier再一起运行，所以计算总时间计算选为从第一个线程创建clone3开始，到最后一个线程退出exit结束

### 2 实验结果

分别实验从1线程到8线程的计算时间，如下表所示

|hart number|runtime (ticks) |
|----------|-----------------------------:|
|8|5,294,357,000|
|4|10,569,638,000|
|2|21,138,661,000|
|1|42,270,441,000|
