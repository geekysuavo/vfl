
/* ensure once-only inclusion. */
#ifndef __VFL_MATRIX_H__
#define __VFL_MATRIX_H__

/* include c library headers. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* include vfl headers. */
#include <vfl/util/vector.h>

/* matrix_disp(): macro function for displaying the contents of matrices.
 */
#define matrix_disp(A) matrix_dispfn(A, #A)

/* Matrix: structure for holding a real matrix.
 */
typedef struct {
  /* data access parameters:
   *  @rows: number of matrix rows.
   *  @cols: number of matrix columns.
   *  @stride: array spacing between elements.
   */
  unsigned int rows, cols;
  unsigned int stride;

  /* @data: array or pointer to matrix elements.
   */
  double *data;
}
Matrix;

/* MatrixView: aliased type used to explicitly note that a given
 * matrix data structure does not own its data array.
 */
typedef Matrix MatrixView;

/* function declarations (util/matrix.c): */

unsigned int matrix_bytes (const unsigned int rows,
                           const unsigned int cols);

void matrix_init (void *addr, const unsigned int rows,
                  const unsigned int cols);

Matrix *matrix_alloc (const unsigned int rows, const unsigned int cols);

void matrix_copy (Matrix *dest, const Matrix *src);

void matrix_copy_row (Vector *dest, const Matrix *src,
                      const unsigned int i);

void matrix_copy_col (Vector *dest, const Matrix *src,
                      const unsigned int j);

void matrix_free (Matrix *A);

MatrixView matrix_view_array (double *data,
                              const unsigned int n1,
                              const unsigned int n2);

VectorView matrix_diag (const Matrix *A);

VectorView matrix_row (const Matrix *A, const unsigned int i);

VectorView matrix_col (const Matrix *A, const unsigned int j);

VectorView matrix_subrow (const Matrix *A, const unsigned int i,
                          const unsigned int offset,
                          const unsigned int n);

VectorView matrix_subcol (const Matrix *A, const unsigned int j,
                          const unsigned int offset,
                          const unsigned int n);

MatrixView matrix_submatrix (const Matrix *A,
                             const unsigned int i1,
                             const unsigned int i2,
                             const unsigned int n1,
                             const unsigned int n2);

double matrix_get (const Matrix *A,
                   const unsigned int i,
                   const unsigned int j);

void matrix_set (Matrix *A, const unsigned int i, const unsigned int j,
                 const double Aij);

void matrix_set_all (Matrix *A, const double Aall);

void matrix_set_ident (Matrix *A);

void matrix_set_zero (Matrix *A);

void matrix_sub (Matrix *A, const Matrix *B);

void matrix_scale (Matrix *A, const double alpha);

void matrix_dispfn (const Matrix *A, const char *str);

#endif /* !__VFL_MATRIX_H__ */

