
/* ensure once-only inclusion. */
#ifndef __VFL_EIGEN_H__
#define __VFL_EIGEN_H__

/* include the matrix, vector, and blas headers. */
#include <vfl/util/matrix.h>
#include <vfl/util/vector.h>
#include <vfl/util/blas.h>

/* function declarations (eigen.c): */

double eigen_upper (const matrix_t *A);

double eigen_minev (const matrix_t *A, matrix_t *B,
                    vector_t *b, vector_t *z);

#endif /* !__VFL_EIGEN_H__ */

