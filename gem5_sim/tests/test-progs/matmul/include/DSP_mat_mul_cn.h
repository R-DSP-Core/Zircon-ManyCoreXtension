/* ======================================================================= */
/* DSP_mat_mul_cn.h -- Perform Matrix Multiplication                       */
/*                     Natural C Implementation                            */
/* ======================================================================= */

#ifndef DSP_MAT_MUL_CN_H_
#define DSP_MAT_MUL_CN_H_ 1

#ifndef restrict
#define restrict __restrict
#endif

void DSP_mat_mul_cn (
    const short *restrict x, int r1, int c1r2,
    const short *restrict y,         int c2,
    int       *restrict r,
    int                   qs
);

/* 多线程版本：计算指定行范围的矩阵乘法 */
void DSP_mat_mul_cn_range (
    const short *restrict x, int r1, int c1r2,
    const short *restrict y,         int c2,
    short       *restrict r,
    int                   qs,
    int                   start_row,
    int                   end_row
);

#endif /* DSP_MAT_MUL_CN_H_ */

