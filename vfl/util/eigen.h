
/* ensure once-only inclusion. */
#ifndef __VFL_EIGEN_H__
#define __VFL_EIGEN_H__

/* include the matrix, vector, and blas headers. */
#include <vfl/util/matrix.h>
#include <vfl/util/vector.h>
#include <vfl/util/blas.h>

/* function declarations (util/eigen.c): */

double eigen_upper (const Matrix *A);

double eigen_minev (const Matrix *A, Matrix *B,
                    Vector *b, Vector *z);

#endif /* !__VFL_EIGEN_H__ */

