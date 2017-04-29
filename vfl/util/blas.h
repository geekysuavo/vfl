
/* ensure once-only inclusion. */
#ifndef __VFL_BLAS_H__
#define __VFL_BLAS_H__

/* include the matrix and vector headers. */
#include <vfl/util/matrix.h>
#include <vfl/util/vector.h>

/* determine whether or not atlas is used. */
#ifdef __VFL_USE_ATLAS
/* include the atlas blas header. */
#include <cblas.h>
#include <clapack.h>
#else
/* define values for cblas enumerations. */
#define CblasNoTrans 111
#define CblasTrans   112
#define CblasUpper   121
#define CblasLower   122
#endif

/* blas_transpose_t: enumeration of all possible ways to transpose
 * a matrix (or not) during calculations.
 */
typedef enum {
  BLAS_NO_TRANS = CblasNoTrans,
  BLAS_TRANS    = CblasTrans
}
blas_transpose_t;

/* blas_triangle_t: enumeration of all possible ways to access
 * a triangular matrix.
 */
typedef enum {
  BLAS_UPPER = CblasUpper,
  BLAS_LOWER = CblasLower
}
blas_triangle_t;

/* function declarations (util/blas.c): */

double blas_dasum (const vector_t *x);

double blas_dnrm2 (const vector_t *x);

double blas_ddot (const vector_t *x, const vector_t *y);

void blas_daxpy (double alpha, const vector_t *x, vector_t *y);

void blas_dscal (double alpha, vector_t *y);

/* --- */

void blas_dgemv (blas_transpose_t trans, double alpha, const matrix_t *A,
                 const vector_t *x, double beta, vector_t *y);

void blas_dtrmv (blas_transpose_t trans, const matrix_t *L,
                 const vector_t *x, vector_t *y);

void blas_dtrsv (blas_triangle_t tri, const matrix_t *L, vector_t *x);

#endif /* !__VFL_BLAS_H__ */

