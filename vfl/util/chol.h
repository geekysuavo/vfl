
/* ensure once-only inclusion. */
#ifndef __VFL_CHOL_H__
#define __VFL_CHOL_H__

/* include the blas header. */
#include <vfl/util/blas.h>

/* function declarations (util/chol.c): */

int chol_decomp (Matrix *A);

int chol_invert (const Matrix *L, Matrix *B);

void chol_solve (const Matrix *L, const Vector *b, Vector *x);

void chol_update (Matrix *L, Vector *x);

int chol_downdate (Matrix *L, Vector *y);

#endif /* !__VFL_CHOL_H__ */

