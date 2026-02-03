ENTRY(_start)

MEMORY
{
    /* L1I SPM: 64KB at 0x80000000 */
    L1I (rwx) : ORIGIN = 0x80000000, LENGTH = 64K
    /* L1D SPM: 64KB at 0x80010000 */
    L1D (rw)  : ORIGIN = 0x80010000, LENGTH = 64K
    /* L1D SPM: 256KB at 0x80020000 */
    L2  (rw)  : ORIGIN = 0x80020000, LENGTH = 256K
}

SECTIONS
{
    . = ORIGIN(L1I);

    /* Code and Read-Only Data go to L1I */
    .text : {
        *(.text*)
    } > L1I

    /* 字符串常量等只读数据也放在 L1I，避免被测试数据覆盖 */
    .rodata : {
        *(.rodata*)
    } > L1I

    /* Initialized Data goes to L1D (and copied to L1D if we had startup code, but simplified here) */
    /* For simplicity in this baremetal test, we put data in L1I too, or L1D if accessible */
    /* Let's put everything in L1I for now since L1I is 0x80000000 where loader writes */
    .data : {
        *(.data*)
        *(.sdata*)
    } > L2

    .bss : {
        *(.bss*)
        *(.sbss*)
    }

    /* Stack at the end of L1D */
    . = ORIGIN(L2) + LENGTH(L2) - 0x100;
    __stack_top = .;
}
