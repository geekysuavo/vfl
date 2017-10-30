
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

/* BlasTranspose: enumeration of all possible ways to transpose
 * a matrix (or not) during calculations.
 */
typedef enum {
  BLAS_NO_TRANS = CblasNoTrans,
  BLAS_TRANS    = CblasTrans
}
BlasTranspose;

/* BlasTriangle: enumeration of all possible ways to access
 * a triangular matrix.
 */
typedef enum {
  BLAS_UPPER = CblasUpper,
  BLAS_LOWER = CblasLower
}
BlasTriangle;

/* function declarations (util/blas.c): */

double blas_dasum (const Vector *x);

double blas_dnrm2 (const Vector *x);

double blas_ddot (const Vector *x, const Vector *y);

void blas_daxpy (double alpha, const Vector *x, Vector *y);

void blas_dscal (double alpha, Vector *y);

/* --- */

void blas_dgemv (BlasTranspose trans, double alpha, const Matrix *A,
                 const Vector *x, double beta, Vector *y);

void blas_dtrmv (BlasTranspose trans, const Matrix *L,
                 const Vector *x, Vector *y);

void blas_dtrsv (BlasTriangle tri, const Matrix *L, Vector *x);

#endif /* !__VFL_BLAS_H__ */

