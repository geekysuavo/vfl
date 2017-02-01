
/* ensure once-only inclusion. */
#ifndef __VFL_CHOL_H__
#define __VFL_CHOL_H__

/* include the matrix, vector, and blas headers. */
#include <vfl/matrix.h>
#include <vfl/vector.h>
#include <vfl/blas.h>

/* function declarations (chol.c): */

int chol_decomp (matrix_t *A);

int chol_invert (const matrix_t *L, matrix_t *B);

void chol_solve (const matrix_t *L, const vector_t *b, vector_t *x);

void chol_update (matrix_t *L, vector_t *x);

int chol_downdate (matrix_t *L, vector_t *y);

#endif /* !__VFL_CHOL_H__ */

