
/* ensure once-only inclusion. */
#ifndef __VFL_EIGEN_H__
#define __VFL_EIGEN_H__

/* include the blas header. */
#include <vfl/util/blas.h>

/* function declarations (util/eigen.c): */

double eigen_upper (const Matrix *A);

double eigen_minev (const Matrix *A, Matrix *B,
                    Vector *b, Vector *z);

#endif /* !__VFL_EIGEN_H__ */

