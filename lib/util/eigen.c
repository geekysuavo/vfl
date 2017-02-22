
/* include the eigenvalue algorithms header. */
#include <vfl/util/eigen.h>

/* eigen_upper(): compute an upper bound on the eigenvalues of
 * a real symmetric positive definite matrix. this uses the
 * gershgorin circle theorm for bounding the spectrum of a
 * square matrix.
 *
 * arguments:
 *  @A: input matrix to compute a bound for.
 *
 * returns:
 *  real number @U such that the largest eigenvalue of the input
 *  matrix is bounded above by @U.
 */
double eigen_upper (const matrix_t *A) {
  /* initialize the upper bound. */
  double ub = -1.0e9;

  /* loop over each row of the matrix. */
  for (unsigned int i = 0; i < A->rows; i++) {
    /* get the current absolute row sum of the matrix. */
    vector_view_t ai = matrix_row(A, i);
    double Ri = blas_dasum(&ai);

    /* replace the absolute diagonal element with the original element. */
    const double aii = matrix_get(A, i, i);
    Ri += aii - fabs(aii);

    /* if the row sum exceeds the current bound, update the bound. */
    if (Ri > ub)
      ub = Ri;
  }

  /* return the computed upper bound. */
  return ub;
}

/* eigen_minev(): compute the smallest eigenvalue of a real symmetric
 * positive definite matrix using the power iteration method.
 *
 * arguments:
 *  @A: input matrix to the computation.
 *  @B: temporary storage for the spectrally shifted matrix.
 *  @b: temporary vector for the eigenvector estimate.
 *  @z: temporary vector for storing products.
 *
 * returns:
 *  min(eig(A)).
 */
double eigen_minev (const matrix_t *A, matrix_t *B,
                    vector_t *b, vector_t *z) {
  /* declare required variables:
   *  @mu: current eigenvalue estimate.
   *  @mu_prev: previous eigenvalue estimate.
   *  @steps: number of power iterations.
   */
  double mu, mu_prev;
  unsigned int steps;

  /* handle the special case of 1x1 matrices. */
  if (A->rows == 1)
    return matrix_get(A, 0, 0);

  /* initialize the temporary structures. */
  matrix_copy(B, A);
  vector_set_all(b, 1.0);

  /* compute the upper bound on the eigenvalues of the matrix. */
  const double evub = eigen_upper(B);

  /* shift the spectrum of the matrix. */
  vector_view_t Bdiag = matrix_diag(B);
  vector_add_const(&Bdiag, -evub);

  /* loop until convergence. */
  steps = 0;
  mu = 0.0;
  do {
    /* store the previous eigenvalue. */
    mu_prev = mu;

    /* hit the eigenvector estimate with the spectrally shifted matrix. */
    blas_dgemv(BLAS_NO_TRANS, 1.0, B, b, 0.0, z);
    const double znrm = blas_dnrm2(z);

    /* if the eigenvector estimate is zero, return the bound. */
    if (znrm == 0.0)
      return evub;

    /* upate the eigenvector estimate. */
    vector_scale(z, 1.0 / znrm);
    vector_copy(b, z);

    /* update the eigenvalue estimate. */
    blas_dgemv(BLAS_NO_TRANS, 1.0, B, b, 0.0, z);
    mu = blas_ddot(b, z);

    /* increment the step count. */
    steps++;
  }
  while (fabs(mu_prev - mu) > 1.0e-6 && steps < 5);

  /* return the minimum eigenvalue. */
  return mu + evub;
}

