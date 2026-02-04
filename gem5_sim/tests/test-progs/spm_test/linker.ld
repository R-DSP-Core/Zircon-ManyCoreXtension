ENTRY(_start)

SECTIONS
{
    /* Code and Read-Only Data go to L1I */
    .text : {
        *(.text*)
        .=ALIGN(4);    
    }

    /* 字符串常量等只读数据也放在 L1I，避免被测试数据覆盖 */
    .rodata : {
        *(.rodata*)
    }

    /* Initialized Data goes to L1D (and copied to L1D if we had startup code, but simplified here) */
    /* For simplicity in this baremetal test, we put data in L1I too, or L1D if accessible */
    /* Let's put everything in L1I for now since L1I is 0x80000000 where loader writes */
    .data : {
        __data_start = .;
        *(.data*)
        *(.sdata*)
        __data_end = .;
    }

    .bss : {
        *(.bss*)
        *(.sbss*)
        .=ALIGN(4);    
    }

    /* Stack at the end of L1D */
    __stack_top = .;
}
