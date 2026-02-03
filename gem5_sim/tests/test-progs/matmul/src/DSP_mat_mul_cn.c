#include "DSP_mat_mul_cn.h"

#ifndef _nassert
#define _nassert(x) ((void)0)
#endif

void DSP_mat_mul_cn (
    const short *restrict x, int r1, int c1r2,
    const short *restrict y,         int c2,
    int       *restrict r,
    int                   qs
)
{
    int i, j, k;
    int sum;

#ifndef NOASSUME
    _nassert(r1 >= 1 && r1 <= 32767);
    _nassert(c1r2 >= 1 && c1r2 <= 32767);
    _nassert(c2 >= 1 && c2 <= 32767);
#endif

    /* -------------------------------------------------------------------- */
    /*  Multiply each row in x by each column in y.  The product of row m   */
    /*  in x and column n in y is placed in position (m,n) in the result.   */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < r1; i++) {
        for (j = 0; j < c2; j++) {
            for (k = 0, sum = 0; k < c1r2; k++)
                sum += x[k + i*c1r2] * y[j + k*c2];
            r[j + i*c2] = sum >> qs;
        }
    }
}

/* 多线程版本：只计算指定行范围 */
void DSP_mat_mul_cn_range (
    const short *restrict x, int r1, int c1r2,
    const short *restrict y,         int c2,
    short       *restrict r,
    int                   qs,
    int                   start_row,
    int                   end_row
)
{
    int i, j, k;
    int sum;

#ifndef NOASSUME
    _nassert(r1 >= 1 && r1 <= 32767);
    _nassert(c1r2 >= 1 && c1r2 <= 32767);
    _nassert(c2 >= 1 && c2 <= 32767);
    _nassert(start_row >= 0 && start_row < r1);
    _nassert(end_row > start_row && end_row <= r1);
#endif

    /* 只计算指定范围的行 */
    for (i = start_row; i < end_row; i++) {
        for (j = 0; j < c2; j++) {
            for (k = 0, sum = 0; k < c1r2; k++)
                sum += x[k + i*c1r2] * y[j + k*c2];
            r[j + i*c2] = sum >> qs;
        }
    }
}
