/*
 * RISC-V 64-bit Startup Code for Bare-metal SPM Test
 */

    .section .text
    .global _start
    .type _start, @function

_start:
    mv      s0, zero
    la      sp, __stack_top
    jal     main
#     /* main 返回后，a0 包含返回值，调用 exit */
#     li      a7, 93          /* SYS_exit */
#     ecall
# 1:  j       1b              /* 死循环保险 */
