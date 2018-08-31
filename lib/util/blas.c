
/* inclue the basic linear algebra subroutines header. */
#include <vfl/util/blas.h>

/* blas_dasum(): compute the sum of absolute values of the elements
 * of a vector.
 *
 * operation:
 *  result <- sum_i |x_i|
 *
 * arguments:
 *  @x: input vector to the sum operation.
 *
 * returns:
 *  sum of absolute values of the vector elements.
 */
double blas_dasum (const Vector *x) {
#ifdef __VFL_USE_ATLAS
  /* use atlas blas. */
  return cblas_dasum(x->len, x->data, x->stride);
#else
  /* initialize the result. */
  double result = 0.0;

  /* compute the sum of the absolute values of the vector elements. */
  for (unsigned int i = 0; i < x->len; i++)
    result += fabs(vector_get(x, i));

  /* return the result. */
  return result;
#endif
}

/* blas_dnrm2(): compute the euclidean norm of a vector, equivalent to
 * the square root of the sum of its squared elements.
 *
 * operation:
 *  result <- ||x||_2 = sqrt(sum(x.^2))
 *
 * arguments:
 *  @x: input vector to the norm operation.
 *
 * returns:
 *  euclidean norm of the input vector.
 */
inline double blas_dnrm2 (const Vector *x) {
#ifdef __VFL_USE_ATLAS
  /* use atlas blas. */
  return cblas_dnrm2(x->len, x->data, x->stride);
#else
  /* compute and return the result. */
  return sqrt(blas_ddot(x, x));
#endif
}

/* blas_ddot(): compute the euclidean inner product between two vectors,
 *
 * operation:
 *  result <- x' * y = sum(x .* y)
 *
 * arguments:
 *  @x: first input vector.
 *  @y: second input vector.
 *
 * returns:
 *  euclidean inner product (dot product) of the two input vectors.
 */
double blas_ddot (const Vector *x, const Vector *y) {
#ifdef __VFL_USE_ATLAS
  /* use atlas blas. */
  return cblas_ddot(x->len, x->data, x->stride, y->data, y->stride);
#else
  /* initialize the result. */
  double result = 0.0;

  /* compute the sum of the products of each pair of vector elements. */
  for (unsigned int i = 0; i < x->len; i++)
    result += vector_get(x, i) * vector_get(y, i);

  /* return the result. */
  return result;
#endif
}

/* blas_daxpy(): compute the element-wise sum of two vectors.
 *
 * operation:
 *  y <- y + alpha x
 *
 * arguments:
 *  @alpha: scale factor to apply to @x.
 *  @x: input vector to the sum.
 *  @y: input and output vector of the sum.
 */
void blas_daxpy (double alpha, const Vector *x, Vector *y) {
#ifdef __VFL_USE_ATLAS
  /* use atlas blas. */
  cblas_daxpy(x->len, alpha, x->data, x->stride, y->data, y->stride);
#else
  /* compute the sum over all vector elements. */
  for (unsigned int i = 0; i < x->len; i++)
    vector_set(y, i, vector_get(y, i) + alpha * vector_get(x, i));
#endif
}

/* blas_dscal(): compute the scaled value of a vector.
 *
 * operation:
 *  y <- alpha y
 *
 * arguments:
 *  @alpha: scale factor to apply to @y.
 *  @y: input and output vector.
 */
void blas_dscal (double alpha, Vector *y) {
#ifdef __VFL_USE_BLAS
  /* use atlas blas. */
  cblas_dscal(y->len, alpha, y->data, y->stride);
#else
  /* compute the scaled value of each vector element. */
  for (unsigned int i = 0; i < y->len; i++)
    vector_set(y, i, vector_get(y, i) * alpha);
#endif
}

/* --- */

/* blas_dgemv(): compute the linear combination of a vector and the product
 * of a dense matrix with a vector.
 *
 * the user is responsible for ensuring that all matrix and vector operands
 * are non-null and of conformal sizes.
 *
 * operation:
 *  y <- alpha A x  + beta y      if trans == BLAS_NO_TRANS
 *  y <- alpha A' x + beta y      if trans == BLAS_TRANS
 *
 * arguments:
 *  @trans: transposition state of the matrix in the product.
 *  @alpha: scale factor for the matrix-vector product.
 *  @A: input matrix to the product operation.
 *  @x: input vector to the product operation
 *  @beta: scale factor for the output vector.
 *  @y: input and output vector to the combined operation.
 */
void blas_dgemv (BlasTranspose trans, double alpha, const Matrix *A,
                 const Vector *x, double beta, Vector *y) {
#ifdef __VFL_USE_ATLAS
  /* use atlas blas. */
  cblas_dgemv(CblasRowMajor, (enum CBLAS_TRANSPOSE) trans,
              A->rows, A->cols, alpha, A->data, A->stride,
              x->data, x->stride, beta,
              y->data, y->stride);
#else
  /* perform: y <- beta y */
  if (beta == 0.0)
    vector_set_zero(y);
  else
    blas_dscal(beta, y);

  /* if the scale factor to the matrix-vector portion is zero, return. */
  if (alpha == 0.0)
    return;

  /* perform the dense multiplication operation. */
  if (trans == BLAS_NO_TRANS) {
    /* perform: y <- y + alpha A x */
    for (unsigned int i = 0; i < A->rows; i++) {
      VectorView ai = matrix_row(A, i);
      const double ax = blas_ddot(&ai, x);
      vector_set(y, i, vector_get(y, i) + alpha * ax);
    }
  }
  else if (trans == BLAS_TRANS) {
    /* perform: y <- y + alpha A' x */
    for (unsigned int j = 0; j < A->cols; j++) {
      VectorView aj = matrix_col(A, j);
      const double ax = blas_ddot(&aj, x);
      vector_set(y, j, vector_get(y, j) + alpha * ax);
    }
  }
#endif
}

/* blas_dtrmv(): compute the product of a lower-triangular matrix
 * with a vector.
 *
 * the user is responsible for ensuring that all matrix and vector operands
 * are non-null and of conformal sizes. only the lower triangle of the
 * input matrix is accessed.
 *
 * operation:
 *  y <- L x       if trans == BLAS_NO_TRANS
 *  y <- L' x      if trans == BLAS_TRANS
 *
 * arguments:
 *  @trans: transposition state of the matrix in the product.
 *  @L: input lower-triangular matrix to the product.
 *  @x: input vector to the product operation.
 *  @y: output vector to the product operation.
 */
void blas_dtrmv (BlasTranspose trans, const Matrix *L,
                 const Vector *x, Vector *y) {
#ifdef __VFL_USE_ATLAS
  /* use atlas blas. */
  vector_copy(y, x);
  cblas_dtrmv(CblasRowMajor, CblasLower, (enum CBLAS_TRANSPOSE) trans,
              CblasNonUnit, L->rows, L->data, L->stride,
              y->data, y->stride);
#else
  /* locally store the problem size. */
  const unsigned int n = x->len;

  /* perform the triangular multiplication operation. */
  if (trans == BLAS_NO_TRANS) {
    /* perform: y <- L x */
    for (unsigned int i = 0; i < n; i++) {
      VectorView li = matrix_subrow(L, i, 0, i + 1);
      const double lx = blas_ddot(&li, x);
      vector_set(y, i, lx);
    }
  }
  else if (trans == BLAS_TRANS) {
    /* perform: y <- L' x */
    for (unsigned int j = 0; j < n; j++) {
      VectorView lj = matrix_subcol(L, j, j, n - j);
      VectorView sj = vector_subvector(x, j, n - j);
      const double lx = blas_ddot(&lj, &sj);
      vector_set(y, j, lx);
    }
  }
#endif
}

/* blas_dtrsv(): perform forward or backward substitution to solve a system
 * of linear equations involving a triangular matrix.
 *
 * the user is responsible for ensuring that all matrix and vector operands
 * are non-null and of conformal sizes. 
 *
 * operation:
 *  x <- inv(tril(A)) x      if tri == BLAS_LOWER
 *  x <- inv(triu(A)) x      if tri == BLAS_UPPER
 *
 * arguments:
 *  @tri: triangle to access in the matrix in the product.
 *  @A: input triangular matrix.
 *  @x: input and output vector.
 */
void blas_dtrsv (BlasTriangle tri, const Matrix *A, Vector *x) {
#ifdef __VFL_USE_ATLAS
  /* use atlas blas. */
  cblas_dtrsv(CblasRowMajor, (enum CBLAS_UPLO) tri,
              CblasNoTrans, CblasNonUnit,
              A->rows, A->data, A->stride,
              x->data, x->stride);
#else
  /* locally store the problem size. */
  const unsigned int n = x->len;

  /* perform the substitution operation. */
  if (tri == BLAS_LOWER) {
    /* lower triangle => forward substitution: x <- inv(tril(A)) x */
    for (unsigned int i = 0; i < n; i++) {
      VectorView li = matrix_subrow(A, i, 0, i);
      const double Lii = vector_get(&li, i);
      double xi = vector_get(x, i);
      xi -= blas_ddot(&li, x);
      vector_set(x, i, xi / Lii);
    }
  }
  else if (tri == BLAS_UPPER) {
    /* upper triangle => backward substitution: x <- inv(triu(A)) x */
    for (unsigned int i = n - 1; i < n; i--) {
      VectorView ui = matrix_subrow(A, i, i + 1, n - i - 1);
      VectorView si = vector_subvector(x, i + 1, n - i - 1);
      const double Uii = matrix_get(A, i, i);
      double xi = vector_get(x, i);
      xi -= blas_ddot(&ui, &si);
      vector_set(x, i, xi / Uii);
    }
  }
#endif
}

