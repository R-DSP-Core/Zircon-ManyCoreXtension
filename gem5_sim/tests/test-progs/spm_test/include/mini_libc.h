/* mini_libc.h - A minimal libc implementation for RISC-V 32/64 */
#ifndef MINI_LIBC_H
#define MINI_LIBC_H

#include <stdarg.h>

/* Standard Integer Types */
typedef signed char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

/* Types */
typedef unsigned long size_t;

/* Standard definitions */
#define NULL ((void*)0)

/* System Calls */
static inline long __syscall3(long n, long _a0, long _a1, long _a2) {
    register long a7 asm("a7") = n;
    register long a0 asm("a0") = _a0;
    register long a1 asm("a1") = _a1;
    register long a2 asm("a2") = _a2;
    asm volatile ("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a7) : "memory");
    return a0;
}

static inline long __syscall1(long n, long _a0) {
    register long a7 asm("a7") = n;
    register long a0 asm("a0") = _a0;
    asm volatile ("ecall" : "+r"(a0) : "r"(a7) : "memory");
    return a0;
}

/* Linux Syscall Numbers (RISC-V) */
#define SYS_write 64
#define SYS_exit  93
#define SYS_clone 220
#define SYS_wait4 260

/* Forward declarations */
void _exit(int status);

/* Clone flags (simplified) */
#define CLONE_VM 0x00000100
#define CLONE_FILES 0x00000400
#define CLONE_SIGHAND 0x00000800
#define CLONE_THREAD 0x00010000
#define CLONE_SYSVSEM 0x00040000
#define CLONE_SETTLS 0x00080000
#define CLONE_PARENT_SETTID 0x00100000
#define CLONE_CHILD_CLEARTID 0x00200000
#define CLONE_DETACHED 0x00400000

/* Thread creation helper */
// RISC-V clone: long clone(unsigned long flags, void *stack, int *parent_tid, unsigned long tls, int *child_tid)
static inline long clone_syscall(
    long (*fn)(void *),
    void *stack,
    int flags,
    void *arg,
    int *ptid,
    int *ctid,
    unsigned long tls
) {
    register long a0 asm("a0") = flags;
    register void *a1 asm("a1") = stack;
    register int *a2 asm("a2") = ptid;
    register unsigned long a3 asm("a3") = tls;
    register int *a4 asm("a4") = ctid;
    register long a7 asm("a7") = SYS_clone;
    
    long ret;
    asm volatile (
        "ecall"
        : "=r"(ret)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a7)
        : "memory"
    );
    
    if (ret == 0) {
        // Child thread: call the function
        long result = fn(arg);
        _exit(result);
    }
    
    return ret; // Parent: return child PID
}

/* Basic Functions */
void _exit(int status) {
    __syscall1(SYS_exit, status);
    while(1); // Should not reach here
}

/* Wait for child process/thread */
static inline long wait4_syscall(long pid, int *status, int options, void *rusage) {
    register long a0 asm("a0") = pid;
    register int *a1 asm("a1") = status;
    register int a2 asm("a2") = options;
    register void *a3 asm("a3") = rusage;
    register long a7 asm("a7") = SYS_wait4;
    
    long ret;
    asm volatile (
        "ecall"
        : "=r"(ret)
        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a7)
        : "memory"
    );
    return ret;
}

size_t strlen(const char *s) {
    const char *p = s;
    while (*p) p++;
    return p - s;
}

void *memcpy(void *dest, const void *src, size_t n) {
    char *d = (char *)dest;
    const char *s = (const char *)src;
    while (n--) *d++ = *s++;
    return dest;
}

void *memset(void *s, int c, size_t n) {
    unsigned char *p = (unsigned char *)s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}

/* Minimal Printf */
static void __putc(char c) {
    __syscall3(SYS_write, 1, (long)&c, 1);
}

static void __puts(const char *s) {
    __syscall3(SYS_write, 1, (long)s, strlen(s));
}

static void __put_hex(unsigned long n) {
    char buffer[17];
    int i = 0;
    if (n == 0) {
        __puts("0");
        return;
    }
    while (n > 0) {
        int d = n % 16;
        buffer[i++] = (d < 10) ? (d + '0') : (d - 10 + 'a');
        n /= 16;
    }
    while (i > 0) __putc(buffer[--i]);
}

static void __put_dec(long n) {
    char buffer[21];
    int i = 0;
    int sign = 0;
    if (n < 0) {
        sign = 1;
        n = -n;
    }
    if (n == 0) {
        __puts("0");
        return;
    }
    while (n > 0) {
        buffer[i++] = (n % 10) + '0';
        n /= 10;
    }
    if (sign) __putc('-');
    while (i > 0) __putc(buffer[--i]);
}

int printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 's': __puts(va_arg(args, const char *)); break;
                case 'd': __put_dec(va_arg(args, int)); break;
                case 'x': __put_hex(va_arg(args, unsigned int)); break;
                case 'l': 
                    if (*(fmt+1) == 'd') { __put_dec(va_arg(args, long)); fmt++; }
                    else if (*(fmt+1) == 'x') { __put_hex(va_arg(args, unsigned long)); fmt++; }
                    break;
                case '%': __putc('%'); break;
                default: __putc('%'); __putc(*fmt); break;
            }
        } else {
            __putc(*fmt);
        }
        fmt++;
    }
    va_end(args);
    return 0;
}

/* Entry Point Wrapper */
/* User code should implement main() */
extern int main();

void _start() {
    /* 由 linker.ld 提供的段边界符号 */
    extern uint8_t __data_start;
    extern uint8_t __data_end;

    /*
     * SE 模式下：加载器会按 ELF 的 VMA 直接把 .data 初始化到目标地址
     * （本工程的 linker.ld 已把 .data 放到 0x80020000 的 L2 SPM）。
     *
     * 因此这里“搬运 .data”通常不需要；但 .bss 必须清零。
     */
    memcpy((uint32_t*) 0x80020000, &__data_start, (size_t)(&__data_end - &__data_start));

    int ret = main();
    _exit(ret);
}

#endif // MINI_LIBC_H

