/********************************************************
 * Kernels to be optimized for the CS:APP Performance Lab
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

/*
 * Please fill in the following team struct
 */
team_t team = {
    "USTC-Z", /* Team name */

    "Yinnan Zhou",                 /* First member full name */
    "yinnanzhou@mail.ustc.edu.cn", /* First member email address */

    "", /* Second member full name (leave blank if none) */
    ""  /* Second member email addr (leave blank if none) */
};

/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/

/*
 * naive_rotate - The naive baseline version of rotate
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel* src, pixel* dst) {
    int i, j;

    for (i = 0; i < dim; i++)
        for (j = 0; j < dim; j++)
            dst[RIDX(dim - 1 - j, i, dim)] = src[RIDX(i, j, dim)];
}

/*
 * rotate - Your current working version of rotate
 * IMPORTANT: This is the version you will be graded on
 */
char rotate_descr[] = "rotate: Current working version";
void rotate(int dim, pixel* src, pixel* dst) {
    naive_rotate(dim, src, dst);
}

char rotate_descr_8[] = "rotate_descr_8";
void rotate_8(int dim, pixel* src, pixel* dst) {
    for (int i = 0; i < dim; i += 8) {
        for (int j = 0; j < dim; j += 8) {
            for (int p = i; p < i + 8; p++) {
                for (int q = j; q < j + 8; q++) {
                    dst[RIDX(dim - 1 - q, p, dim)] = src[RIDX(p, q, dim)];
                }
            }
        }
    }
}

char rotate_descr_8_unrolled[] = "rotate_descr_8_unrolled";
void rotate_8_unrolled(int dim, pixel* src, pixel* dst) {
    int dst_init = (dim - 1) * dim;
    int dst_offset = dim + 8;
    int src_offset = dim * 8;
    dst += dst_init;
    for (int i = 0; i < dim; i += 8) {
        for (int j = 0; j < dim; j++) {
            for (int k = 0; k < 8; ++k) {
                *dst = *src;
                src += dim;
                dst++;
            }

            src -= src_offset - 1;
            dst -= dst_offset;
        }
        dst += dst_init + dst_offset;
        src += src_offset - dim;
    }
}

char rotate_descr_16[] = "rotate_descr_16";
void rotate_16(int dim, pixel* src, pixel* dst) {
    for (int i = 0; i < dim; i += 16) {
        for (int j = 0; j < dim; j += 16) {
            for (int p = i; p < i + 16; p++) {
                for (int q = j; q < j + 16; q++) {
                    dst[RIDX(dim - 1 - q, p, dim)] = src[RIDX(p, q, dim)];
                }
            }
        }
    }
}

char rotate_descr_16_unrolled[] = "rotate_descr_16_unrolled";
void rotate_16_unrolled(int dim, pixel* src, pixel* dst) {
    int dst_init = (dim - 1) * dim;
    int dst_offset = dim + 16;
    int src_offset = dim * 16;
    dst += dst_init;
    for (int i = 0; i < dim; i += 16) {
        for (int j = 0; j < dim; j++) {
            for (int k = 0; k < 16; ++k) {
                *dst = *src;
                src += dim;
                dst++;
            }

            src -= src_offset - 1;
            dst -= dst_offset;
        }
        dst += dst_init + dst_offset;
        src += src_offset - dim;
    }
}

char rotate_descr_32[] = "rotate_descr_32";
void rotate_32(int dim, pixel* src, pixel* dst) {
    for (int i = 0; i < dim; i += 32) {
        for (int j = 0; j < dim; j += 32) {
            for (int p = i; p < i + 32; p++) {
                for (int q = j; q < j + 32; q++) {
                    dst[RIDX(dim - 1 - q, p, dim)] = src[RIDX(p, q, dim)];
                }
            }
        }
    }
}

char rotate_descr_32_unrolled[] = "rotate_descr_32_unrolled";
void rotate_32_unrolled(int dim, pixel* src, pixel* dst) {
    int dst_init = (dim - 1) * dim;
    int dst_offset = dim + 32;
    int src_offset = dim * 32;
    dst += dst_init;
    for (int i = 0; i < dim; i += 32) {
        for (int j = 0; j < dim; j++) {
            for (int k = 0; k < 32; ++k) {
                *dst = *src;
                src += dim;
                dst++;
            }

            src -= src_offset - 1;
            dst -= dst_offset;
        }
        dst += dst_init + dst_offset;
        src += src_offset - dim;
    }
}

/*********************************************************************
 * register_rotate_functions - Register all of your different versions
 *     of the rotate kernel with the driver by calling the
 *     add_rotate_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.
 *********************************************************************/

void register_rotate_functions() {
    add_rotate_function(&naive_rotate, naive_rotate_descr);
    // add_rotate_function(&rotate, rotate_descr);
    // add_rotate_function(&rotate_8, rotate_descr_8);
    // add_rotate_function(&rotate_8_unrolled, rotate_descr_8_unrolled);
    // add_rotate_function(&rotate_16, rotate_descr_16);
    // add_rotate_function(&rotate_16_unrolled, rotate_descr_16_unrolled);
    // add_rotate_function(&rotate_32, rotate_descr_32);
    add_rotate_function(&rotate_32_unrolled, rotate_descr_32_unrolled);
    /* ... Register additional test functions here */
}

/***************
 * SMOOTH KERNEL
 **************/

/***************************************************************
 * Various typedefs and helper functions for the smooth function
 * You may modify these any way you like.
 **************************************************************/

/* A struct used to compute averaged pixel value */
typedef struct {
    int red;
    int green;
    int blue;
    int num;
} pixel_sum;

/* Compute min and max of two integers, respectively */
static int min(int a, int b) {
    return (a < b ? a : b);
}
static int max(int a, int b) {
    return (a > b ? a : b);
}

/*
 * initialize_pixel_sum - Initializes all fields of sum to 0
 */
static void initialize_pixel_sum(pixel_sum* sum) {
    sum->red = sum->green = sum->blue = 0;
    sum->num = 0;
    return;
}

/*
 * accumulate_sum - Accumulates field values of p in corresponding
 * fields of sum
 */
static void accumulate_sum(pixel_sum* sum, pixel p) {
    sum->red += (int)p.red;
    sum->green += (int)p.green;
    sum->blue += (int)p.blue;
    sum->num++;
    return;
}

/*
 * assign_sum_to_pixel - Computes averaged pixel value in current_pixel
 */
static void assign_sum_to_pixel(pixel* current_pixel, pixel_sum sum) {
    current_pixel->red = (unsigned short)(sum.red / sum.num);
    current_pixel->green = (unsigned short)(sum.green / sum.num);
    current_pixel->blue = (unsigned short)(sum.blue / sum.num);
    return;
}

/*
 * avg - Returns averaged pixel value at (i,j)
 */
static pixel avg(int dim, int i, int j, pixel* src) {
    int ii, jj;
    pixel_sum sum;
    pixel current_pixel;

    initialize_pixel_sum(&sum);
    for (ii = max(i - 1, 0); ii <= min(i + 1, dim - 1); ii++)
        for (jj = max(j - 1, 0); jj <= min(j + 1, dim - 1); jj++)
            accumulate_sum(&sum, src[RIDX(ii, jj, dim)]);

    assign_sum_to_pixel(&current_pixel, sum);
    return current_pixel;
}

/******************************************************
 * Your different versions of the smooth kernel go here
 ******************************************************/

/*
 * naive_smooth - The naive baseline version of smooth
 */
char naive_smooth_descr[] = "naive_smooth: Naive baseline implementation";
void naive_smooth(int dim, pixel* src, pixel* dst) {
    int i, j;

    for (i = 0; i < dim; i++)
        for (j = 0; j < dim; j++)
            dst[RIDX(i, j, dim)] = avg(dim, i, j, src);
}

/*
 * smooth - Your current working version of smooth.set
 * IMPORTANT: This is the version you will be graded on
 */

static void avg_corner(int index,
                       pixel* src,
                       pixel* dst,
                       int offset1,
                       int offset2,
                       int offset3) {
    dst[index].red = (src[index].red + src[index + offset1].red +
                      src[index + offset2].red + src[index + offset3].red) /
                     4;
    dst[index].blue = (src[index].blue + src[index + offset1].blue +
                       src[index + offset2].blue + src[index + offset3].blue) /
                      4;
    dst[index].green =
        (src[index].green + src[index + offset1].green +
         src[index + offset2].green + src[index + offset3].green) /
        4;
}

static void avg_edge(int index,
                     pixel* src,
                     pixel* dst,
                     int offset1,
                     int offset2,
                     int offset3,
                     int offset4,
                     int offset5) {
    dst[index].red = (src[index].red + src[index + offset1].red +
                      src[index + offset2].red + src[index + offset3].red +
                      src[index + offset4].red + src[index + offset5].red) /
                     6;
    dst[index].blue = (src[index].blue + src[index + offset1].blue +
                       src[index + offset2].blue + src[index + offset3].blue +
                       src[index + offset4].blue + src[index + offset5].blue) /
                      6;
    dst[index].green =
        (src[index].green + src[index + offset1].green +
         src[index + offset2].green + src[index + offset3].green +
         src[index + offset4].green + src[index + offset5].green) /
        6;
}

static void arg_ordinary(int index, pixel* src, pixel* dst, int dim) {
    dst[index].red = (src[index].red + src[index - 1].red + src[index + 1].red +
                      src[index + dim - 1].red + src[index + dim].red +
                      src[index + dim + 1].red + src[index - dim - 1].red +
                      src[index - dim].red + src[index - dim + 1].red) /
                     9;
    dst[index].blue =
        (src[index].blue + src[index - 1].blue + src[index + 1].blue +
         src[index + dim - 1].blue + src[index + dim].blue +
         src[index + dim + 1].blue + src[index - dim - 1].blue +
         src[index - dim].blue + src[index - dim + 1].blue) /
        9;
    dst[index].green =
        (src[index].green + src[index - 1].green + src[index + 1].green +
         src[index + dim - 1].green + src[index + dim].green +
         src[index + dim + 1].green + src[index - dim - 1].green +
         src[index - dim].green + src[index - dim + 1].green) /
        9;
}

char smooth_descr[] = "my_smooth: Improved and unrolled";
void smooth(int dim, pixel* src, pixel* dst) {
    // corner nodes
    avg_corner(0, src, dst, 1, dim, dim + 1);         // top-left
    avg_corner(dim - 1, src, dst, -1, dim - 1, dim);  // top-right
    avg_corner(dim * (dim - 1), src, dst, 1, -dim,
               -dim + 1);  // bottom -left
    avg_corner(dim * dim - 1, src, dst, -1, -dim - 1,
               -dim);  // bottom-right

    // edge nodes
    for (int i = 1; i <= dim - 2; i++) {
        avg_edge(i, src, dst, -1, 1, dim, dim - 1, dim + 1);  // top
        avg_edge(dim * (dim - 1) + i, src, dst, -1, 1, -dim, -dim - 1,
                 -dim + 1);  // bottom
        avg_edge(i * dim, src, dst, -dim, dim, -dim + 1, 1,
                 dim + 1);  // left
        avg_edge(dim * (i + 1) - 1, src, dst, -dim, dim, -dim - 1, -1,
                 dim - 1);  // right
    }

    // ordinary nodes
    for (int i = 1; i <= dim - 2; i++) {
        for (int j = 1; j <= dim - 2; j++) {
            arg_ordinary(i * dim + j, src, dst, dim);
        }
    }
}

/*********************************************************************
 * register_smooth_functions - Register all of your different versions
 *     of the smooth kernel with the driver by calling the
 *     add_smooth_function() for each test function.  When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.
 *********************************************************************/

void register_smooth_functions() {
    add_smooth_function(&naive_smooth, naive_smooth_descr);
    add_smooth_function(&smooth, smooth_descr);
    /* ... Register additional test functions here */
}
