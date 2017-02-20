
/* include the model header. */
#include <vfl/model.h>

/* sigfn(): compute the standard logistic function.
 *
 * computation:
 *  sigma(xi) = (1 + e^(-x))^(-1)
 */
static inline double sigfn (const double xi) {
  /* compute and return the function value. */
  return 1.0 / (1.0 + exp(-xi));
}

/* ellfn(): compute the ell-function used by the logistic bound.
 *
 * computation:
 *  ell(xi) = (4 xi)^(-1) tanh(xi / 2)
 */
static inline double ellfn (const double xi) {
  /* compute and return the function value. */
  return tanh(0.5 * xi) / (4.0 * xi);
}

/* --- */

/* model_vfc(): allocate a new variational feature classification model.
 *
 * arguments:
 *  @nu: prior absolute precision of the weights.
 *
 * returns:
 *  newly allocated and initialized vfc model.
 */
model_t *model_vfc (const double nu) {
  /* allocate a new variational feature model. */
  model_t *mdl = model_alloc();
  if (!mdl)
    return NULL;

  /* set the function pointers. */
  mdl->bound = vfc_bound;
  mdl->predict = vfc_predict;
  mdl->infer = vfc_infer;
  mdl->update = vfc_update;
  mdl->grad = vfc_gradient;

  /* attempt to set the prior parameters. */
  if (!model_set_nu(mdl, nu)) {
    model_free(mdl);
    return NULL;
  }

  /* return the new model. */
  return mdl;
}

/* vfc_bound(): return the lower bound of a vfc model.
 *  - see model_bound_fn() for more information.
 */
double vfc_bound (const model_t *mdl) {
  /* initialize the computation. */
  double bound = 0.0;

  /* include the complexity term. */
  for (unsigned int k = 0; k < mdl->K; k++)
    bound -= log(matrix_get(mdl->L, k, k));

  /* include the data fit term. */
  vector_view_t b = vector_subvector(mdl->tmp, 0, mdl->K);
  blas_dtrmv(BLAS_TRANS, 1.0, mdl->L, mdl->wbar, 0.0, &b);
  bound += 0.5 * blas_ddot(&b, &b);

  /* include the logistic terms. */
  for (unsigned int i = 0; i < mdl->dat->N; i++) {
    const double xi = vector_get(mdl->xi, i);
    bound += sigfn(xi) - 0.5 * xi + ellfn(xi) * xi * xi;
  }

  /* return the computed result. */
  return bound;
}

/* vfc_predict(): return the prediction of a vfc model.
 *  - see model_predict_fn() for more information.
 */
int vfc_predict (const model_t *mdl, const vector_t *x,
                 double *mean, double *var) {
  /* initialize the predicted mean. */
  double p = 0.0;

  /* loop over the terms of the inner product. */
  for (unsigned int j = 0, i = 0; j < mdl->M; j++) {
    for (unsigned int k = 0; k < mdl->factors[j]->K; k++, i++) {
      /* include the current contribution form the inner product. */
      p += vector_get(mdl->wbar, i) * model_mean(mdl, x, j, k);
    }
  }

  /* pass the inner product through the logistic function. */
  p = sigfn(p);

  /* store the results and return success. */
  *mean = p;
  *var = p * (1.0 - p);
  return 1;
}

/* vfc_infer(): perform complete inference in a vfc model.
 *  - see model_infer_fn() for more information.
 */
int vfc_infer (model_t *mdl) {
  /* gain access to the dataset structure members. */
  const unsigned int N = mdl->dat->N;
  matrix_t *X = mdl->dat->X;
  vector_t *y = mdl->dat->y;
  double yi, xi;

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

      /* compute the contribution of each observation. */
      for (unsigned int i = 0; i < N; i++) {
        x = matrix_row(X, i);
        yi = vector_get(y, i);
        hk += (2.0 * yi - 1.0) * model_mean(mdl, &x, j1, k1);
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
            xi = vector_get(mdl->xi, i);
            gkk += 2.0 * ellfn(xi) * model_var(mdl, &x, j1, j2, k1, k2);
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

  /* update the weight means. */
  chol_solve(mdl->L, mdl->h, mdl->wbar);
  vector_scale(mdl->wbar, 0.5);

  /* update the weight covariances. */
  chol_invert(mdl->L, mdl->Sigma);

  /* update the logistic parameters. */
  for (unsigned int i = 0; i < N; i++) {
    /* initialize the parameter computation. */
    x = matrix_row(X, i);
    xi = 0.0;

    /* loop over the first trace dimension. */
    for (unsigned int j1 = 0, i1 = 0; j1 < mdl->M; j1++) {
      for (unsigned int k1 = 0; k1 < mdl->factors[j1]->K; k1++, i1++) {
        /* loop over the second trace dimension. */
        for (unsigned int j2 = 0, i2 = 0; j2 < mdl->M; j2++) {
          for (unsigned int k2 = 0; k2 < mdl->factors[j2]->K; k2++, i2++) {
            /* include the current contribution from the trace. */
            xi += (matrix_get(mdl->Sigma, i1, i2) +
                   vector_get(mdl->wbar, i1) *
                   vector_get(mdl->wbar, i2)) *
                  model_var(mdl, &x, j1, j2, k1, k2);
          }
        }
      }
    }

    /* store the computed logistic parameter. */
    vector_set(mdl->xi, i, sqrt(xi));
  }

  /* return success. */
  return 1;
}

/* vfc_update(): perform efficient low-rank inference in a vfc model.
 *  - see model_update_fn() for more information.
 */
int vfc_update (model_t *mdl, const unsigned int j) {
  /* get the weight offset and count of the current factor. */
  const unsigned int k0 = model_weight_idx(mdl, j, 0);
  const unsigned int K = mdl->factors[j]->K;

  /* gain access to the dataset structure members. */
  const unsigned int N = mdl->dat->N;
  matrix_t *X = mdl->dat->X;
  vector_t *y = mdl->dat->y;
  vector_view_t x;
  double yi, xi;

  /* prepare for low-rank adjustment. */
  model_weight_adjust_init(mdl, j);

  /* loop over the weights of the current factor. */
  for (unsigned int k = 0; k < K; k++) {
    /* initialize the projection subvector element. */
    double hk = 0.0;

    /* compute the contributions of each observation. */
    for (unsigned int i = 0; i < N; i++) {
      x = matrix_row(X, i);
      yi = vector_get(y, i);
      hk += (2.0 * yi - 1.0) * model_mean(mdl, &x, j, k);
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
          xi = vector_get(mdl->xi, i);
          gkk += 2.0 * ellfn(xi) * model_var(mdl, &x, j, j2, k, k2);
        }

        /* store the precision submatrix element. */
        matrix_set(mdl->Sinv, k0 + k, i2 + k2, gkk);
        matrix_set(mdl->Sinv, i2 + k2, k0 + k, gkk);
      }

      /* move to the next precision submatrix. */
      i2 += K2;
    }
  }

  /* include the diagonal term into the weight precisions. */
  for (unsigned int k = 0; k < K; k++) {
    double gkk = matrix_get(mdl->Sinv, k0 + k, k0 + k);
    matrix_set(mdl->Sinv, k0 + k, k0 + k, gkk + mdl->nu);
  }

  /* perform low-rank adjustment. */
  if (!model_weight_adjust(mdl, j))
    return 0;

  /* update the weight means. */
  chol_solve(mdl->L, mdl->h, mdl->wbar);
  vector_scale(mdl->wbar, 0.5);

  /* update the logistic parameters. */
  for (unsigned int i = 0; i < N; i++) {
    /* initialize the parameter computation. */
    x = matrix_row(X, i);
    xi = 0.0;

    /* loop over the first trace dimension. */
    for (unsigned int j1 = 0, i1 = 0; j1 < mdl->M; j1++) {
      for (unsigned int k1 = 0; k1 < mdl->factors[j1]->K; k1++, i1++) {
        /* loop over the second trace dimension. */
        for (unsigned int j2 = 0, i2 = 0; j2 < mdl->M; j2++) {
          for (unsigned int k2 = 0; k2 < mdl->factors[j2]->K; k2++, i2++) {
            /* include the current contribution from the trace. */
            xi += (matrix_get(mdl->Sigma, i1, i2) +
                   vector_get(mdl->wbar, i1) *
                   vector_get(mdl->wbar, i2)) *
                  model_var(mdl, &x, j1, j2, k1, k2);
          }
        }
      }
    }

    /* store the computed logistic parameter. */
    vector_set(mdl->xi, i, sqrt(xi));
  }

  /* return success. */
  return 1;
}

/* vfc_gradient(): return the gradient of a single factor in a vfc model.
 *  - see model_gradient_fn() for more information.
 */
int vfc_gradient (const model_t *mdl, const unsigned int i,
                  const unsigned int j, vector_t *grad) {
  /* determine the weight index offset of the current factor. */
  const unsigned int k0 = model_weight_idx(mdl, j, 0);

  /* gain access to the factor weight count. */
  const factor_t *fj = mdl->factors[j];
  const unsigned int K = fj->K;

  /* gain access to the specified observation. */
  vector_view_t x = matrix_row(mdl->dat->X, i);
  const double y = vector_get(mdl->dat->y, i);

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
                         wk * vector_get(mdl->wbar, k0 + kk);

      /* include the second-order contribution. */
      factor_diff_var(fj, &x, k, kk, &g);
      blas_daxpy(-0.5 * wwT, &g, grad);
    }

    /* include the first-order contribution. */
    factor_diff_mean(fj, &x, k, &g);
    blas_daxpy(wk * y, &g, grad);

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
                           wk * vector_get(mdl->wbar, i2 + k2);

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

