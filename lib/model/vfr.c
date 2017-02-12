
/* include the model header. */
#include <vfl/model.h>

/* model_vfr(): allocate a new variational feature regression model.
 *
 * arguments:
 *  @alpha0: prior shape parameter for the noise precision.
 *  @beta0: prior rate parameter for the noise precision.
 *  @nu: prior relative precision of the weights.
 *
 * returns:
 *  newly allocated and initialized vfr model.
 */
model_t *model_vfr (const double alpha0,
                    const double beta0,
                    const double nu) {
  /* allocate a variational feature model. */
  model_t *mdl = model_alloc();
  if (!mdl)
    return NULL;

  /* set the function pointers. */
  mdl->bound = vfr_bound;
  mdl->predict = vfr_predict;
  mdl->infer = vfr_infer;
  mdl->update = vfr_update;
  mdl->grad = vfr_gradient;

  /* attempt to set the prior parameters. */
  if (!model_set_alpha0(mdl, alpha0) ||
      !model_set_beta0(mdl, beta0) ||
      !model_set_nu(mdl, nu)) {
    model_free(mdl);
    return NULL;
  }

  /* return the new model. */
  return mdl;
}

/* vfr_bound(): return the lower bound of a vfr model.
 *  - see model_bound_fn() for more information.
 */
double vfr_bound (const model_t *mdl) {
  /* initialize the computation. */
  double elbo = 0.0;

  /* include the complexity term. */
  for (unsigned int k = 0; k < mdl->K; k++)
    elbo -= log(matrix_get(mdl->L, k, k));

  /* include the data fit term. */
  elbo -= mdl->alpha * log(mdl->beta);

  /* return the computed result. */
  return elbo;
}

/* vfr_predict(): return the prediction of a vfr model.
 *  - see model_predict_fn() for more information.
 */
int vfr_predict (const model_t *mdl, const vector_t *x,
                 double *mean, double *var) {
  /* initialize the predicted mean. */
  double mu = 0.0;

  /* loop over the terms of the inner product. */
  for (unsigned int j = 0, i = 0; j < mdl->M; j++) {
    for (unsigned int k = 0; k < mdl->factors[j]->K; k++, i++) {
      /* include the current contribution from the inner product. */
      mu += vector_get(mdl->wbar, i) * model_mean(mdl, x, j, k);
    }
  }

  /* compute the expected noise variance and initialize
   * the predicted variance.
   */
  const double tauinv = mdl->beta / (mdl->alpha - 1.0);
  double eta = tauinv - mu * mu;

  /* loop over the first trace dimension. */
  for (unsigned int j1 = 0, i1 = 0; j1 < mdl->M; j1++) {
    for (unsigned int k1 = 0; k1 < mdl->factors[j1]->K; k1++, i1++) {
      /* loop over the second trace dimension. */
      for (unsigned int j2 = 0, i2 = 0; j2 < mdl->M; j2++) {
        for (unsigned int k2 = 0; k2 < mdl->factors[j2]->K; k2++, i2++) {
          /* include the current contribution from the trace. */
          eta += (matrix_get(mdl->Sigma, i1, i2) +
                  vector_get(mdl->wbar, i1) *
                  vector_get(mdl->wbar, i2)) *
                 model_var(mdl, x, j1, j2, k1, k2);
        }
      }
    }
  }

  /* store the results and return success. */
  *mean = mu;
  *var = eta;
  return 1;
}

/* vfr_infer(): perform complete inference in a vfr model.
 *  - see model_infer_fn() for more information.
 */
int vfr_infer (model_t *mdl) {
  /* gain access to the dataset structure members. */
  const unsigned int N = mdl->dat->N;
  matrix_t *X = mdl->dat->X;
  vector_t *y = mdl->dat->y;
  double yi;

  /* declare views into the weight precisions and the projections. */
  matrix_view_t G;
  vector_view_t h;
  vector_view_t x;

  /* loop over the factors. */
  for (unsigned int j1 = 0, i1 = 0; j1 < mdl->M; j1++) {
    /* get the projection subvector. */
    const unsigned int K1 = mdl->factors[j1]->K;
    h = vector_subvector(mdl->h, i1, K1);

    /* loop over the weights of the current factor. */
    for (unsigned int k1 = 0; k1 < K1; k1++) {
      /* initialize the projection subvector element. */
      double hk = 0.0;

      /* compute the contributions of each observation. */
      for (unsigned int i = 0; i < N; i++) {
        x = matrix_row(X, i);
        yi = vector_get(y, i);
        hk += yi * model_mean(mdl, &x, j1, k1);
      }

      /* store the projection subvector element. */
      vector_set(&h, k1, hk);

      /* loop again over the factors. */
      for (unsigned int j2 = 0, i2 = 0; j2 < mdl->M; j2++) {
        /* get the precision submatrix. */
        const unsigned int K2 = mdl->factors[j2]->K;
        G = matrix_submatrix(mdl->Sinv, i1, i2, K1, K2);

        /* loop again over the factor weights. */
        for (unsigned int k2 = 0; k2 < K2; k2++) {
          /* initialize the precision submatrix element. */
          double gkk = 0.0;

          /* compute the contributions of each observation. */
          for (unsigned int i = 0; i < N; i++) {
            x = matrix_row(X, i);
            gkk += model_var(mdl, &x, j1, j2, k1, k2);
          }

          /* store the precision submatrix element. */
          matrix_set(&G, k1, k2, gkk);
        }

        /* move to the next precision submatrix. */
        i2 += K2;
      }
    }

    /* move to the next projection subvector. */
    i1 += K1;
  }

  /* include the diagonal term into the weight precisions. */
  vector_view_t Gdiag = matrix_diag(mdl->Sinv);
  vector_add_const(&Gdiag, mdl->nu);

  /* compute the cholesky decomposition of the weight precisions. */
  matrix_copy(mdl->L, mdl->Sinv);
  chol_decomp(mdl->L);

  /* update the weight means and covariances. */
  chol_solve(mdl->L, mdl->h, mdl->wbar);
  chol_invert(mdl->L, mdl->Sigma);

  /* compute the data and model inner products. */
  vector_view_t z = vector_subvector(mdl->tmp, 0, mdl->K);
  blas_dtrmv(BLAS_TRANS, 1.0, mdl->L, mdl->wbar, 0.0, &z);
  const double wSw = blas_ddot(&z, &z);
  const double yy = blas_ddot(y, y);

  /* update the noise shape and rate. */
  mdl->alpha = mdl->alpha0 + 0.5 * (double) N;
  mdl->beta = mdl->beta0 + 0.5 * (yy - wSw);

  /* return success. */
  return 1;
}

/* vfr_update(): perform efficient low-rank inference in a vfr model.
 *  - see model_update_fn() for more information.
 */
int vfr_update (model_t *mdl, const unsigned int j) {
  /* determine the weight index offset of the current factor. */
  const unsigned int k0 = model_weight_idx(mdl, j, 0);

  /* gain access to the factor weight count. */
  const unsigned int K = mdl->factors[j]->K;

  /* gain access to the dataset structure members. */
  const unsigned int N = mdl->dat->N;
  matrix_t *X = mdl->dat->X;
  vector_t *y = mdl->dat->y;
  double yi;

  /* declare views into the update and downdate matrices. */
  vector_view_t u, v, x, z;
  matrix_view_t U, V;

  /* create the vector view for covariance matrix updates and downdates. */
  double *ptr = mdl->tmp->data;
  z = vector_view_array(ptr, mdl->K);
  ptr += mdl->K + mdl->P;

  /* create the matrix view for cholesky updates. */
  U = matrix_view_array(ptr, K, mdl->K);
  ptr += K * mdl->K;

  /* create the matrix view for cholesky downdates. */
  V = matrix_view_array(ptr, K, mdl->K);

  /* copy the initial row values of the precision matrix. */
  for (unsigned int k = 0; k < K; k++) {
    u = matrix_row(&U, k);
    matrix_copy_row(&u, mdl->Sinv, k0 + k);
  }

  /* loop over the weights of the current factor. */
  for (unsigned int k = 0; k < K; k++) {
    /* initialize the projection subvector element. */
    double hk = 0.0;

    /* compute the contributions of each observation. */
    for (unsigned int i = 0; i < N; i++) {
      x = matrix_row(X, i);
      yi = vector_get(y, i);
      hk += yi * model_mean(mdl, &x, j, k);
    }

    /* store the projection subvector element. */
    vector_set(mdl->h, k0 + k, hk);

    /* loop again over the factors. */
    for (unsigned int j2 = 0, i2 = 0; j2 < mdl->M; j2++) {
      /* get the precision submatrix. */
      const unsigned int K2 = mdl->factors[j2]->K;

      /* loop again over the factor weights. */
      for (unsigned int k2 = 0; k2 < K2; k2++) {
        /* initialize the precision submatrix element. */
        double gkk = 0.0;

        /* compute the contributions of each observation. */
        for (unsigned int i = 0; i < N; i++) {
          x = matrix_row(X, i);
          gkk += model_var(mdl, &x, j, j2, k, k2);
        }

        /* store the precision submatrix element. */
        matrix_set(mdl->Sinv, k0 + k, i2 + k2, gkk);
        matrix_set(mdl->Sinv, i2 + k2, k0 + k, gkk);
      }

      /* move to the next precision submatrix. */
      i2 += K2;
    }
  }

  /* copy the final row values of the precision matrix. */
  for (unsigned int k = 0; k < K; k++) {
    v = matrix_row(&V, k);
    matrix_copy_row(&v, mdl->Sinv, k0 + k);
  }

  /* compute the difference between the precision matrix rows. */
  matrix_sub(&V, &U);

  /* adjust the row differences for use in rank-1 updates. */
  for (unsigned int k = 0; k < K; k++) {
    /* scale the main diagonal element by one-half, and zero
     * all off-diagonals that have already been updated.
     */
    v = matrix_row(&V, k);
    vector_set(&v, k0 + k, 0.5 * vector_get(&v, k0 + k));
    for (unsigned int kk = 0; kk < k; kk++)
      vector_set(&v, k0 + kk, 0.0);
  }

  /* transform the row differences into symmetric updates and downdates. */
  for (unsigned int k = 0; k < K; k++) {
    /* get views of the update and downdate row vectors. */
    u = matrix_row(&U, k);
    v = matrix_row(&V, k);

    /* compute the symmetrization constants. */
    const double vnrm = blas_dnrm2(&v);
    const double alpha = sqrt(vnrm / 2.0);
    const double beta = 1.0 / vnrm;

    /* symmetrize the vectors. */
    for (unsigned int i = 0; i < mdl->K; i++) {
      /* get the elements of the selector and asymmetric update. */
      const double ui = (i == k0 + k ? 1.0 : 0.0);
      const double vi = vector_get(&v, i);

      /* compute the elements of the symmetric update/downdate. */
      const double xi = alpha * (ui + beta * vi);
      const double yi = alpha * (ui - beta * vi);

      /* store the elements back into their vectors. */
      vector_set(&u, i, xi);
      vector_set(&v, i, yi);
    }
  }

  /* apply the updates. */
  for (unsigned int k = 0; k < K; k++) {
    /* update the cholesky factors. */
    u = matrix_row(&U, k);
    matrix_copy_row(&z, &U, k);
    chol_update(mdl->L, &z);

    /* update the covariance matrix. */
    blas_dgemv(BLAS_NO_TRANS, 1.0, mdl->Sigma, &u, 0.0, &z);
    double zudot = blas_ddot(&z, &u);
    zudot = 1.0 / (1.0 + zudot);
    for (unsigned int i = 0; i < mdl->K; i++)
      for (unsigned int j = 0; j < mdl->K; j++)
        matrix_set(mdl->Sigma, i, j,
          matrix_get(mdl->Sigma, i, j) -
          zudot * vector_get(&z, i) *
                  vector_get(&z, j));
  }

  /* apply the downdates. */
  for (unsigned int k = 0; k < K; k++) {
    /* downdate the cholesky factors. */
    v = matrix_row(&V, k);
    matrix_copy_row(&z, &V, k);
    chol_downdate(mdl->L, &z);

    /* downdate the covariance matrix. */
    blas_dgemv(BLAS_NO_TRANS, 1.0, mdl->Sigma, &v, 0.0, &z);
    double zvdot = blas_ddot(&z, &v);
    zvdot = 1.0 / (1.0 - zvdot);
    for (unsigned int i = 0; i < mdl->K; i++)
      for (unsigned int j = 0; j < mdl->K; j++)
        matrix_set(mdl->Sigma, i, j,
          matrix_get(mdl->Sigma, i, j) +
          zvdot * vector_get(&z, i) *
                  vector_get(&z, j));
  }

  /* update the weight means. */
  chol_solve(mdl->L, mdl->h, mdl->wbar);

  /* compute the data and model inner products. */
  blas_dtrmv(BLAS_TRANS, 1.0, mdl->L, mdl->wbar, 0.0, &z);
  const double wSw = blas_ddot(&z, &z);
  const double yy = blas_ddot(y, y);

  /* update the noise shape and rate. */
  mdl->alpha = mdl->alpha0 + 0.5 * (double) N;
  mdl->beta = mdl->beta0 + 0.5 * (yy - wSw);

  /* if the update failed to yield reasonable values, perform
   * a full re-inference.
   */
  if (!isfinite(mdl->beta))
    return vfr_infer(mdl);

  /* return success. */
  return 1;
}

/* vfr_gradient(): return the gradient of a single factor in a vfr model.
 *  - see model_gradient_fn() for more information.
 */
int vfr_gradient (const model_t *mdl, const unsigned int i,
                  const unsigned int j, vector_t *grad) {
  /* determine the weight index offset of the current factor. */
  const unsigned int k0 = model_weight_idx(mdl, j, 0);

  /* gain access to the factor weight count. */
  const factor_t *fj = mdl->factors[j];
  const unsigned int K = fj->K;

  /* gain access to the specified observation. */
  vector_view_t x = matrix_row(mdl->dat->X, i);
  const double y = vector_get(mdl->dat->y, i);

  /* compute the expected noise precision. */
  const double tau = mdl->alpha / mdl->beta;

  /* create the vector view for individual gradient terms. */
  vector_view_t g = vector_subvector(mdl->tmp, mdl->K, grad->len);

  /* loop over the weights of the current factor. */
  for (unsigned int k = 0; k < K; k++) {
    /* compute the weight first moment. */
    const double wk = vector_get(mdl->wbar, k0 + k);

    /* loop over the other weights of the current factor. */
    for (unsigned int kk = 0; kk < K; kk++) {
      /* compute the weight second moment. */
      const double wwT = matrix_get(mdl->Sigma, k0 + k, k0 + kk) +
                         tau * wk * vector_get(mdl->wbar, k0 + kk);

      /* include the second-order contribution. */
      factor_diff_var(fj, &x, k, kk, &g);
      blas_daxpy(-0.5 * wwT, &g, grad);
    }

    /* include the first-order contribution. */
    factor_diff_mean(fj, &x, k, &g);
    blas_daxpy(tau * wk * y, &g, grad);

    /* loop over the other factors. */
    for (unsigned int j2 = 0, i2 = 0; j2 < mdl->M; j2++) {
      /* get the weight count of the other factor. */
      const unsigned int K2 = mdl->factors[j2]->K;

      /* the diagonal second-order contribution was already included. */
      if (j2 == j) {
        i2 += K2;
        continue;
      }

      /* loop over the other factor weights. */
      for (unsigned int k2 = 0; k2 < K2; k2++) {
        /* compute the weight second moment. */
        const double wwT = matrix_get(mdl->Sigma, k0 + k, i2 + k2) +
                           tau * wk * vector_get(mdl->wbar, i2 + k2);

        /* include the off-diagonal second-order contribution. */
        const double E2 = factor_mean(mdl->factors[j2], &x, k2);
        blas_daxpy(-wwT * E2, &g, grad);
      }

      /* move to the next set of weights. */
      i2 += K2;
    }
  }

  /* return success. */
  return 1;
}

