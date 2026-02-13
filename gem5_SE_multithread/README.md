Please perform the following modifications to your gem5 code, and the * is mandatory for a statically-linked C++ pthread program

modified syscall emulation functions:
    ~/gem5/src/sim/syscall_emul.cc: writevFunc, modified to detect 32-bit case

added syscall emulation mapping:
    ~/gem5/src/arch/riscv/linux/se_workload.cc: 
        SyscallDescTable<SEWorkload::SyscallABI32> EmuLinux::syscallDescs32   |   inserted mapping item {291, "statx",        statxFunc<RiscvLinux32>}
                                                                              |   inserted mapping item {293, "rseq",         ignoreWithEnosysFunc} *
                                                                              |   inserted mapping item {422, "futex_time64", futexFunc<RiscvLinux32>} *
                                                                              |   inserted mapping item {435, "clone3",       clone3Func<RiscvLinux32>} *
added syscall struct:
    ~/gem5/src/arch/riscv/linux/linux.hh: 
        class RiscvLinux32 : public RiscvLinux, public OpenFlagTable<RiscvLinux32>   |   inserted tgt_statx struct, concrete struct is described in glibc's struct_statx.h
                                                                                     |   inserted tgt_clone_args, concrete struct is described in glibc's clone3.h *

modified stack allocation strategy:
    ~/gem5/src/arch/riscv/process.cc:
        RiscvProcess32::RiscvProcess32(const ProcessParams &params, loader::ObjectFile *objFile) : RiscvProcess(params, objFile)   |   modified mmap_end to td::min<Addr>(0x40000000L, next_thread_stack_base), the concrete large stack allocation issue is commited at https://github.com/gem5/gem5/commit/25523e73a4f3af5dcbe237c8f0211f43648ac589

NOTE: concrete supported syscall information is in unistd_32.h. stack allocation issue, together with statx/writev syscall emulations are related to dynamic link, which is not fully resolved in SE mode, and the following experiment are proceeded with static linked C++ pthread program.

NOTE: concrete supported syscall information is in unistd_32.h.

modified original python class config:
    ~/gem5/src/cpu/o3/FUPool.py:
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
                RdWrPort(), # ld/st pipeline 1
                RdWrPort()  # ld/st pipeline 2
            ] *

