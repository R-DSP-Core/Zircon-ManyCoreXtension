// #include <stdint.h>
#include "mini_libc.h"
#include "DSP_mat_mul_cn.h"

#define MAX_THREADS 32
#define STACK_SIZE (128 * 1024) // 128KB per thread stack (增大栈大小)

// 线程参数结构
typedef struct {
    
    int thread_id;
    int num_threads;
    int mat_n;
    short *mat_a;
    short *mat_b;
    short *mat_c;
    int start_row;
    int end_row;
    volatile int *done_flag; // 完成标志
} thread_arg_t;

// 线程工作函数：使用 DSP_mat_mul_cn_range 计算矩阵的一部分行
long thread_worker(void *arg_ptr) {
    thread_arg_t *arg = (thread_arg_t *)arg_ptr;
    
    // 调用 DSP 库函数计算指定行范围
    DSP_mat_mul_cn_range(
        arg->mat_a,           // x
        arg->mat_n,           // r1 (总行数)
        arg->mat_n,           // c1r2 (列数)
        arg->mat_b,           // y
        arg->mat_n,           // c2 (列数)
        arg->mat_c,           // r (结果)
        0,                    // qs (shift amount)
        arg->start_row,       // start_row
        arg->end_row          // end_row
    );
    
    // 标记完成
    *(arg->done_flag) = 1;
    
    return 0;
}

// Bootloader主函数
int main(int argc, const char *argv[]){
    // Mat Mul buffers
    static int mat_n = 64; // 减小矩阵规模以进行流水线追踪
    static short mat_a[1024 * 1024];
    static short mat_b[1024 * 1024];
    static short mat_c[1024 * 1024];
    
    // ========================================================================
    // 单线程版本（注释保留用于对比）
    // ========================================================================
    /*
    // 初始化矩阵
    for(int i = 0; i < mat_n * mat_n; i++) {
        mat_a[i] = 10;
        mat_b[i] = 10;
        mat_c[i] = 0;
    }
    
    printf("Starting single-threaded matrix multiplication (%dx%d)...\n", mat_n, mat_n);
    
    // 直接调用 DSP 库函数（单线程，整个矩阵）
    DSP_mat_mul_cn(mat_a, mat_n, mat_n, mat_b, mat_n, mat_c, 0);
    
    printf("Matrix multiplication completed!\n");
    printf("Result C[0][0] = %d (expected: %d)\n", 
           mat_c[0], mat_n * 10 * 10);
    
    return 0;
    */
    // ========================================================================
    // 多线程版本（当前使用）
    // ========================================================================
    
    // 线程相关
    static char thread_stacks[MAX_THREADS][STACK_SIZE];
    static thread_arg_t thread_args[MAX_THREADS];
    static long thread_pids[MAX_THREADS];
    static volatile int thread_done[MAX_THREADS]; // 线程完成标志
    int num_threads = 31; // 使用 8 个线程对应 8 个核
    
    // 初始化完成标志
    for (int i = 0; i < num_threads; i++) {
        thread_done[i] = 0;
    }
    
    // 初始化矩阵
    for(int i = 0; i < mat_n * mat_n; i++) {
        mat_a[i] = 10; // 使用较小的值避免溢出
        mat_b[i] = 10;
        mat_c[i] = 0;
    }

    printf("Starting multi-threaded matrix multiplication (%dx%d, %d threads)...\n", 
           mat_n, mat_n, num_threads);
    
    // 分配任务：每个线程计算一部分行
    int rows_per_thread = mat_n / num_threads;
    int extra_rows = mat_n % num_threads;
    
    int current_row = 0;
    for (int t = 0; t < num_threads; t++) {
        thread_args[t].thread_id = t;
        thread_args[t].num_threads = num_threads;
        thread_args[t].mat_n = mat_n;
        thread_args[t].mat_a = mat_a;
        thread_args[t].mat_b = mat_b;
        thread_args[t].mat_c = mat_c;
        thread_args[t].start_row = current_row;
        thread_args[t].done_flag = &thread_done[t];
        
        // 分配行数（如果有余数，前面的线程多分配一行）
        int rows = rows_per_thread + (t < extra_rows ? 1 : 0);
        thread_args[t].end_row = current_row + rows;
        current_row += rows;
        
        // 准备栈（栈从高地址向低地址增长，16字节对齐）
        // 留一些空间避免栈溢出
        char *stack_top = thread_stacks[t] + STACK_SIZE - 16;
        // RISC-V 要求栈指针 16 字节对齐
        void *stack = (void*)((unsigned long)stack_top & ~0xFUL);
        
        // 创建线程
        // 使用 CLONE_THREAD 标志创建线程（共享地址空间）
        int flags = CLONE_VM | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_SYSVSEM;
        long pid = clone_syscall(
            thread_worker,
            stack,
            flags,
            &thread_args[t],
            NULL, // ptid
            NULL, // ctid
            0     // tls
        );
        
        if (pid < 0) {
            printf("Failed to create thread %d (error: %ld), continuing with %d threads\n", t, pid, t);
            num_threads = t; // 调整实际使用的线程数
            break;
        }
        
        thread_pids[t] = pid;
        printf("Created thread %d (PID: %ld), rows [%d, %d)\n", 
               t, pid, thread_args[t].start_row, thread_args[t].end_row);
    }
    
    printf("Successfully created %d threads\n", num_threads);
    
    // 等待所有线程完成（使用忙等待）
    printf("Waiting for all threads to complete...\n");
    int all_done = 0;
    while (!all_done) {
        all_done = 1;
        for (int t = 0; t < num_threads; t++) {
            if (!thread_done[t]) {
                all_done = 0;
                break;
            }
        }
        // 简单的延迟（避免 CPU 100%）
        for (volatile int i = 0; i < 1000; i++);
    }
    printf("All threads completed!\n");
    
    // 验证结果（检查第一行第一个元素）
    printf("Matrix multiplication completed!\n");
    printf("mat_c[0] = %d\n", mat_c[0]);
    printf("mat_c[1] = %d\n", mat_c[1]);
    printf("mat_c[2] = %d\n", mat_c[2]);
    printf("mat_c[3] = %d\n", mat_c[3]);
    printf("mat_c[4] = %d\n", mat_c[4]);
    printf("mat_c[5] = %d\n", mat_c[5]);
    printf("mat_c[6] = %d\n", mat_c[6]);
    printf("mat_c[7] = %d\n", mat_c[7]);
    printf("mat_c[8] = %d\n", mat_c[8]);
    printf("mat_c[9] = %d\n", mat_c[9]);
    return 0;
}
