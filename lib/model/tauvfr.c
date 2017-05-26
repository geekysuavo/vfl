
/* include the model header. */
#include <vfl/model.h>

/* tauvfr_bound(): return the lower bound of a fixed-tau vfr model.
 *  - see model_bound_fn() for more information.
 */
MODEL_BOUND (tauvfr) {
  /* initialize the computation. */
  const double tau = mdl->tau;
  double bound = 0.0;

  /* include the complexity term. */
  for (unsigned int k = 0; k < mdl->K; k++)
    bound -= log(matrix_get(mdl->L, k, k));

  /* include the data fit term. */
  vector_view_t b = vector_subvector(mdl->tmp, 0, mdl->K);
  blas_dtrmv(BLAS_TRANS, mdl->L, mdl->wbar, &b);
  bound += 0.5 * tau * blas_ddot(&b, &b);

  /* return the computed result. */
  return bound;
}

/* tauvfr_predict(): return the prediction of a fixed-tau vfr model.
 *  - see model_predict_fn() for more information.
 */
MODEL_PREDICT (tauvfr) {
  /* initialize the predicted mean. */
  double mu = 0.0;

  /* loop over the terms of the inner product. */
  for (unsigned int j = 0, i = 0; j < mdl->M; j++) {
    for (unsigned int k = 0; k < mdl->factors[j]->K; k++, i++) {
      /* include the current contribution from the inner product. */
      mu += vector_get(mdl->wbar, i) * model_mean(mdl, x, p, j, k);
    }
  }

  /* compute the expected noise variance and initialize
   * the predicted variance.
   */
  const double tauinv = 1.0 / mdl->tau;
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
                 model_var(mdl, x, p, j1, j2, k1, k2);
        }
      }
    }
  }

  /* store the results and return success. */
  *mean = mu;
  *var = eta;
  return 1;
}

/* tauvfr_infer(): perform complete inference in a fixed-tau vfr model.
 *  - see model_infer_fn() for more information.
 */
MODEL_INFER (tauvfr) {
  /* gain access to the dataset structure members. */
  const unsigned int N = mdl->dat->N;
  data_t *dat = mdl->dat;
  datum_t *di;

  /* declare views into the weight precisions and the projections. */
  matrix_view_t G;
  vector_view_t h;

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
        di = data_get(dat, i);
        hk += di->y * model_mean(mdl, di->x, di->p, j1, k1);
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
            di = data_get(dat, i);
            gkk += model_var(mdl, di->x, di->p, j1, j2, k1, k2);
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

  /* return success. */
  return 1;
}

/* tauvfr_update(): perform efficient low-rank inference in a
 * fixed-tau vfr model.
 *  - see model_update_fn() for more information.
 */
MODEL_UPDATE (tauvfr) {
  /* get the weight offset and count of the current factor. */
  const unsigned int k0 = model_weight_idx(mdl, j, 0);
  const unsigned int K = mdl->factors[j]->K;

  /* gain access to the dataset structure members. */
  const unsigned int N = mdl->dat->N;
  data_t *dat = mdl->dat;
  datum_t *di;

  /* prepare for low-rank adjustment. */
  model_weight_adjust_init(mdl, j);

  /* loop over the weights of the current factor. */
  for (unsigned int k = 0; k < K; k++) {
    /* initialize the projection subvector element. */
    double hk = 0.0;

    /* compute the contributions of each observation. */
    for (unsigned int i = 0; i < N; i++) {
      di = data_get(dat, i);
      hk += di->y * model_mean(mdl, di->x, di->p, j, k);
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
          di = data_get(dat, i);
          gkk += model_var(mdl, di->x, di->p, j, j2, k, k2);
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

  /* return success. */
  return 1;
}

/* tauvfr_gradient(): return the gradient of a single factor in a
 * fixed-tau vfr model.
 *  - see model_gradient_fn() for more information.
 */
MODEL_GRADIENT (tauvfr) {
  /* determine the weight index offset of the current factor. */
  const unsigned int k0 = model_weight_idx(mdl, j, 0);

  /* gain access to the factor weight count. */
  const factor_t *fj = mdl->factors[j];
  const unsigned int K = fj->K;

  /* gain access to the specified observation. */
  datum_t *di = data_get(mdl->dat, i);
  const unsigned int p = di->p;
  const vector_t *x = di->x;
  const double y = di->y;

  /* gain access to the fixed noise precision. */
  const double tau = mdl->tau;

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
      factor_diff_var(fj, x, p, k, kk, &g);
      blas_daxpy(-0.5 * wwT, &g, grad);
    }

    /* include the first-order contribution. */
    factor_diff_mean(fj, x, p, k, &g);
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
        const double E2 = factor_mean(mdl->factors[j2], x, p, k2);
        blas_daxpy(-wwT * E2, &g, grad);
      }

      /* move to the next set of weights. */
      i2 += K2;
    }
  }

  /* return success. */
  return 1;
}

/* tauvfr_meanfield(): return the coefficients required for an
 * assumed-density mean-field update of a factor in a
 * fixed-tau vfr model.
 *  - see model_meanfield_fn() for more information.
 */
MODEL_MEANFIELD (tauvfr) {
  /* get the weight offset and count of the current factor. */
  const unsigned int k0 = model_weight_idx(mdl, j, 0);
  const unsigned int K = mdl->factors[j]->K;

  /* gain access to the dataset structure members. */
  datum_t *dat = mdl->dat->data + i;
  const unsigned int M = mdl->M;

  /* gain access to the fixed noise precision. */
  const double tau = mdl->tau;

  /* create views into the factor weight means and covariances. */
  vector_view_t wk = vector_subvector(mdl->wbar, k0, K);
  matrix_view_t Sk = matrix_submatrix(mdl->Sigma, k0, k0, K, K);

  /* loop over the weights of the current factor. */
  for (unsigned int k = 0; k < K; k++) {
    /* compute and store the contribution. */
    const double bk = tau * dat->y * vector_get(&wk, k);
    vector_set(b, k, bk);
  }

  /* loop over the model factors. */
  for (unsigned int j2 = 0, i2 = 0; j2 < M; j2++) {
    /* get the number of other-factor weights. */
    const unsigned int K2 = mdl->factors[j2]->K;

    /* exclude the factor being updated. */
    if (j2 == j) {
      i2 += K2;
      continue;
    }

    /* loop over the weights of the other factor. */
    for (unsigned int k2 = 0; k2 < K2; k2++) {
      /* get the other factor mean. */
      const double phi2 = model_mean(mdl, dat->x, dat->p, j2, k2);

      /* loop over the weights of the current factor. */
      for (unsigned int k = 0; k < K; k++) {
        /* compute the weight second moment. */
        const double w2 = matrix_get(mdl->Sigma, k0 + k, i2 + k2) +
                          vector_get(mdl->wbar, k0 + k) *
                          vector_get(mdl->wbar, i2 + k2);

        /* update the matrix element accordingly. */
        const double bk = vector_get(b, k);
        vector_set(b, k, bk - tau * w2 * phi2);
      }
    }

    /* move to the next set of other-factor weights. */
    i2 += K2;
  }

  /* compute second-order contributions. */
  for (unsigned int k = 0; k < K; k++) {
    for (unsigned int k2 = 0; k2 < K; k2++) {
      /* compute the current matrix element. */
      const double bkk = -0.5 * tau * (matrix_get(&Sk, k, k2) +
                                       vector_get(&wk, k) *
                                       vector_get(&wk, k2));

      /* store the result. */
      matrix_set(B, k, k2, bkk);
    }
  }

  /* return success. */
  return 1;
}

/* tauvfr_set_tau(): set the noise precision of a fixed-tau vfr model.
 *
 * arguments:
 *  @mdl: model structure pointer.
 *  @tau: noise precision value.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int tauvfr_set_tau (model_t *mdl, const double tau) {
  /* check the input pointer and value. */
  if (!mdl || tau <= 0.0)
    return 0;

  /* set the noise precision. */
  mdl->tau = tau;

  /* set the noise parameters to 'spoof' a fixed precision. */
  mdl->alpha = 1.0e6;
  mdl->beta = 1.0e6 / tau;

  /* return success. */
  return 1;
}

/* tauvfr_properties: array of accessible fixed-tau vfr
 * model object properties.
 */
static object_property_t tauvfr_properties[] = {
  MODEL_PROP_BASE,
  MODEL_PROP_TAU,
  MODEL_PROP_NU,
  { NULL, NULL, NULL }
};

/* tauvfr_methods: array of callable fixed-tau vfr model object methods.
 */
static object_method_t tauvfr_methods[] = {
  MODEL_METHOD_BASE,
  { NULL, NULL }
};

/* tauvfr_type: model type structure for fixed-tau
 * variational feature regression.
 */
static model_type_t tauvfr_type = {
  { /* base: */
    "tauvfr",                                    /* name      */
    sizeof(model_t),                             /* size      */

    (object_init_fn) model_init,                 /* init      */
    NULL,                                        /* copy      */
    (object_free_fn) model_free,                 /* free      */
    NULL,                                        /* test      */
    NULL,                                        /* cmp       */

    NULL,                                        /* add       */
    NULL,                                        /* sub       */
    NULL,                                        /* mul       */
    NULL,                                        /* div       */
    NULL,                                        /* pow       */

    NULL,                                        /* get       */
    NULL,                                        /* set       */
    tauvfr_properties,                           /* props     */
    tauvfr_methods                               /* methods   */
  },

  NULL,                                          /* init      */
  tauvfr_bound,                                  /* bound     */
  tauvfr_predict,                                /* predict   */
  tauvfr_infer,                                  /* infer     */
  tauvfr_update,                                 /* update    */
  tauvfr_gradient,                               /* gradient  */
  tauvfr_meanfield                               /* meanfield */
};

/* vfl_model_tauvfr: address of the tauvfr_type structure. */
const model_type_t *vfl_model_tauvfr = &tauvfr_type;

