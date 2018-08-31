
/* ensure once-only inclusion. */
#ifndef __VFL_MATRIX_H__
#define __VFL_MATRIX_H__

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
  size_t rows, cols;
  size_t stride;

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

size_t matrix_bytes (size_t rows, size_t cols);

void matrix_init (void *addr, size_t rows, size_t cols);

Matrix *matrix_alloc (size_t rows, size_t cols);

void matrix_copy (Matrix *dest, const Matrix *src);

void matrix_copy_row (Vector *dest, const Matrix *src, size_t i);

void matrix_copy_col (Vector *dest, const Matrix *src, size_t j);

void matrix_free (Matrix *A);

MatrixView matrix_view_array (double *data, size_t n1, size_t n2);

VectorView matrix_diag (const Matrix *A);

VectorView matrix_row (const Matrix *A, size_t i);

VectorView matrix_col (const Matrix *A, size_t j);

VectorView matrix_subrow (const Matrix *A, size_t i,
                          size_t offset, size_t n);

VectorView matrix_subcol (const Matrix *A, size_t j,
                          size_t offset, size_t n);

MatrixView matrix_submatrix (const Matrix *A,
                             size_t i1, size_t i2,
                             size_t n1, size_t n2);

double matrix_get (const Matrix *A, size_t i, size_t j);

void matrix_set (Matrix *A, size_t i, size_t j, double Aij);

void matrix_set_all (Matrix *A, double Aall);

void matrix_set_ident (Matrix *A);

void matrix_set_zero (Matrix *A);

void matrix_sub (Matrix *A, const Matrix *B);

void matrix_scale (Matrix *A, double alpha);

void matrix_dispfn (const Matrix *A, const char *str);

#endif /* !__VFL_MATRIX_H__ */

