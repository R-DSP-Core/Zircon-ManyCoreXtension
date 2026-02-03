/*
 * SPM DMA Test Program
 *
 * This program tests the L1D SPM with DMA controller by:
 * 1. Preparing test data in DDR
 * 2. Using DMA to transfer data from DDR to L1D SPM
 * 3. Verifying the transferred data
 * 4. Reporting test results
 */

#include "mini_libc.h"
// mini_libc.h 已定义 uint32_t, uint64_t 等类型，不需要系统头文件

// ============================================================================
// Memory Map
// ============================================================================

// L1D SPM
#define L1D_SPM_BASE    0x80010000UL
#define L1D_SPM_SIZE    (64 * 1024)
#define L1D_SPM_DATA    L1D_SPM_BASE
#define IDMA_BASE     0x80060000UL

// MMR Registers
#define DMA_SRC_ADDR    (IDMA_BASE + 0x00)
#define DMA_DST_ADDR    (IDMA_BASE + 0x04)
#define DMA_SIZE        (IDMA_BASE + 0x08)
#define DMA_CMD         (IDMA_BASE + 0x0C)
#define DMA_STATUS      (IDMA_BASE + 0x10)

// L2 SPM
#define L2_SPM_BASE     0x80020000UL

// L1I SPM (Main Memory for Code)
#define L1I_SPM_BASE    0x80000000UL

int a = 123;
int b = 456;
int c = 789;
int d = 666;


// ============================================================================
// DMA Control Functions
// ============================================================================

// Start DMA transfer
void dma_start(uint32_t src, uint32_t dst, uint32_t size) {
    volatile uint32_t *src_reg = (volatile uint32_t*)DMA_SRC_ADDR;
    volatile uint32_t *dst_reg = (volatile uint32_t*)DMA_DST_ADDR;
    volatile uint32_t *size_reg = (volatile uint32_t*)DMA_SIZE;
    volatile uint32_t *cmd_reg = (volatile uint32_t*)DMA_CMD;
    volatile uint32_t *status_reg = (volatile uint32_t*)DMA_STATUS;

    *src_reg = src;
    *dst_reg = dst;
    *size_reg = size;
    *cmd_reg = 1;  // Start DMA

}

// Wait for DMA to complete
void dma_wait() {
    volatile uint32_t *status_reg = (volatile uint32_t*)DMA_STATUS;

    printf("status: %d\n", *status_reg);
    while (*status_reg != 2) {
        // Busy wait
    }
}

// ============================================================================
// Test Functions
// ============================================================================

// Test 1: DMA from L1I SPM (Code/Data) to L1D SPM
int test_l1i_to_l1d() {
    printf("\n[Test 1] DMA from L1I SPM to L1D SPM\n");

    // Prepare test data in L1I (Data section)
    // We use an offset to avoid overwriting code, assuming code is small (< 32KB)
    uint32_t *l1i_data = (uint32_t*)(L1I_SPM_BASE + 0x8000); 
    const int test_size = 1024;  // 1KB

    printf("  Preparing test data in L1I at 0x%lx...\n", (unsigned long)l1i_data);
    for (int i = 0; i < test_size / 4; i++) {
        l1i_data[i] = 0xDEAD0000 + i;
    }

    // Start DMA transfer
    printf("  Starting DMA transfer (L1I -> L1D)...\n");
    dma_start((uint64_t)l1i_data, L1D_SPM_DATA, test_size);

    // Wait for completion
    printf("  Waiting for DMA to complete...\n");
    dma_wait();
    printf("  DMA transfer completed!\n");

    // Verify data in SPM
    printf("  Verifying data in L1D SPM...\n");
    uint32_t *spm_data = (uint32_t*)L1D_SPM_DATA;
    int errors = 0;
    for (int i = 0; i < test_size / 4; i++) {
        if (spm_data[i] != (0xDEAD0000 + i)) {
            printf("  ERROR at offset %d: expected 0x%x, got 0x%x\n",
                   i * 4, 0xDEAD0000 + i, spm_data[i]);
            errors++;
            if (errors >= 10) break;  // Limit error output
        }
    }

    if (errors == 0) {
        printf("  [PASS] All data verified correctly!\n");
        return 0;
    } else {
        printf("  [FAIL] Found %d errors\n", errors);
        return 1;
    }
}

// Test 2: DMA from L2 SPM to L1D SPM
int test_l2_to_l1() {
    printf("\n[Test 2] DMA from L2 SPM to L1D SPM\n");
    
    // Prepare test data in L2 SPM
    volatile uint32_t *l1_data = (volatile uint32_t*)L1D_SPM_DATA;
    volatile uint32_t *l2_data = (volatile uint32_t*)L2_SPM_BASE;
    const int test_size = 16;  // 512 bytes
    for(int i = 0; i < test_size; i++) {
        // l2_data[i] = 666;
        printf("Before idma transfer, l1d_spm_data in addr %lx = %x\n", L1D_SPM_DATA + i, l1_data[i]);
    }

    // Start DMA transfer
    printf("  Starting DMA transfer (L2 SPM -> L1D SPM)...\n");
    dma_start(L2_SPM_BASE, L1D_SPM_DATA, test_size);

    // Wait for completion
    printf("  Waiting for DMA to complete...\n");
    dma_wait();
    printf("  DMA transfer completed!\n");

    for(int i = 0; i < test_size; i++) {
        // l2_data[i] = 666;
        printf("After idma transfer, l1d_spm_data in addr %lx = %x\n", L1D_SPM_DATA + i, l1_data[i]);
    }

    return 1;
}

// Main function
int main() {
    printf("========================================\n");
    printf("SPM DMA Test Suite\n");
    printf("========================================\n");
    printf("Memory Map:\n");
    printf("  L1D SPM: 0x%lx - 0x%lx\n", L1D_SPM_BASE, L1D_SPM_BASE + L1D_SPM_SIZE);
    printf("  L2 SPM:  0x%lx\n", L2_SPM_BASE);
    printf("  L1I SPM: 0x%lx (Code)\n", L1I_SPM_BASE);
    printf("========================================\n");


    int total_tests = 0;
    int passed_tests = 0;

    // Run tests
    // total_tests++;
    // if (test_l1i_to_l1d() == 0) passed_tests++;
    // int i =100;
    // uint32_t *l2_test = (uint32_t*)L2_SPM_BASE;

    // while(i>0)
    // {
    //     printf("l2 test = %d\n",l2_test[0]);
    //     i--;
    // }
    total_tests++;
    // 从L2 SPM往L1D SPM搬运数据
    // 链接时程序text段就已经放入了L1I SPM
    if (test_l2_to_l1() == 0) passed_tests++;

    // // Print summary
    // printf("\n========================================\n");
    // printf("Test Summary: %d/%d tests passed\n", passed_tests, total_tests);
    // printf("========================================\n");

    // return (passed_tests == total_tests) ? 0 : 1;
}
