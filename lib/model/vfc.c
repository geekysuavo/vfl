
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

/* vfc_bound(): return the lower bound of a vfc model.
 *  - see model_bound_fn() for more information.
 */
MODEL_BOUND (vfc) {
  /* initialize the computation. */
  double bound = 0.0;

  /* include the complexity term. */
  for (unsigned int k = 0; k < mdl->K; k++)
    bound -= log(matrix_get(mdl->L, k, k));

  /* include the data fit term. */
  vector_view_t b = vector_subvector(mdl->tmp, 0, mdl->K);
  blas_dtrmv(BLAS_TRANS, mdl->L, mdl->wbar, &b);
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
MODEL_PREDICT (vfc) {
  /* initialize the predicted mean. */
  double rho = 0.0;

  /* loop over the terms of the inner product. */
  for (unsigned int j = 0, i = 0; j < mdl->M; j++) {
    for (unsigned int k = 0; k < mdl->factors[j]->K; k++, i++) {
      /* include the current contribution form the inner product. */
      rho += vector_get(mdl->wbar, i) * model_mean(mdl, x, p, j, k);
    }
  }

  /* pass the inner product through the logistic function. */
  rho = sigfn(rho);

  /* store the results and return success. */
  *mean = rho;
  *var = rho * (1.0 - rho);
  return 1;
}

/* vfc_infer(): perform complete inference in a vfc model.
 *  - see model_infer_fn() for more information.
 */
MODEL_INFER (vfc) {
  /* gain access to the dataset structure members. */
  const unsigned int N = mdl->dat->N;
  data_t *dat = mdl->dat;
  datum_t *di;
  double xi;

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

      /* compute the contribution of each observation. */
      for (unsigned int i = 0; i < N; i++) {
        di = data_get(dat, i);
        hk += (2.0 * di->y - 1.0) * model_mean(mdl, di->x, di->p, j1, k1);
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
            xi = vector_get(mdl->xi, i);
            gkk += 2.0 * ellfn(xi) * model_var(mdl, di->x, di->p,
                                               j1, j2, k1, k2);
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
  blas_dscal(0.5, mdl->wbar);

  /* update the weight covariances. */
  chol_invert(mdl->L, mdl->Sigma);

  /* update the logistic parameters. */
  for (unsigned int i = 0; i < N; i++) {
    /* initialize the parameter computation. */
    di = data_get(dat, i);
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
                  model_var(mdl, di->x, di->p, j1, j2, k1, k2);
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
MODEL_UPDATE (vfc) {
  /* get the weight offset and count of the current factor. */
  const unsigned int k0 = model_weight_idx(mdl, j, 0);
  const unsigned int K = mdl->factors[j]->K;

  /* gain access to the dataset structure members. */
  const unsigned int N = mdl->dat->N;
  data_t *dat = mdl->dat;
  datum_t *di;
  double xi;

  /* prepare for low-rank adjustment. */
  model_weight_adjust_init(mdl, j);

  /* loop over the weights of the current factor. */
  for (unsigned int k = 0; k < K; k++) {
    /* initialize the projection subvector element. */
    double hk = 0.0;

    /* compute the contributions of each observation. */
    for (unsigned int i = 0; i < N; i++) {
      di = data_get(dat, i);
      hk += (2.0 * di->y - 1.0) * model_mean(mdl, di->x, di->p, j, k);
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
          xi = vector_get(mdl->xi, i);
          gkk += 2.0 * ellfn(xi) * model_var(mdl, di->x, di->p,
                                             j, j2, k, k2);
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
  blas_dscal(0.5, mdl->wbar);

  /* update the logistic parameters. */
  for (unsigned int i = 0; i < N; i++) {
    /* initialize the parameter computation. */
    di = data_get(dat, i);
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
                  model_var(mdl, di->x, di->p, j1, j2, k1, k2);
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
MODEL_GRADIENT (vfc) {
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
      factor_diff_var(fj, x, p, k, kk, &g);
      blas_daxpy(-0.5 * wwT, &g, grad);
    }

    /* include the first-order contribution. */
    factor_diff_mean(fj, x, p, k, &g);
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

/* --- */

/* vfc_properties: array of accessible vfc model object properties.
 */
static object_property_t vfc_properties[] = {
  MODEL_PROP_BASE,
  MODEL_PROP_NU,
  { NULL, NULL, NULL }
};

/* vfc_methods: array of callable vfc model object methods.
 */
static object_method_t vfc_methods[] = {
  MODEL_METHOD_BASE,
  { NULL, NULL }
};

/* vfc_type: model type structure for variational feature classification.
 */
static model_type_t vfc_type = {
  { /* base: */
    "vfc",                                       /* name      */
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
    vfc_properties,                              /* props     */
    vfc_methods                                  /* methods   */
  },

  NULL,                                          /* init      */
  vfc_bound,                                     /* bound     */
  vfc_predict,                                   /* predict   */
  vfc_infer,                                     /* infer     */
  vfc_update,                                    /* update    */
  vfc_gradient,                                  /* gradient  */
  NULL                                           /* meanfield */
};

/* vfl_model_vfc: address of the vfc_type structure. */
const model_type_t *vfl_model_vfc = &vfc_type;

