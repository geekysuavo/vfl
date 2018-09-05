
/* include the vfl header. */
#include <vfl/vfl.h>

/* VFR: structure for holding vfr models.
 */
typedef struct {
  /* model superclass. */
  Model super;

  /* subclass struct members. */
}
VFR;

/* define documentation strings: */

PyDoc_STRVAR(
  VFR_doc,
"VFR() -> VFR object\n"
"\n");

PyDoc_STRVAR(
  VFR_getset_alpha0_doc,
"Prior noise shape (read/write)\n"
"\n");

PyDoc_STRVAR(
  VFR_getset_beta0_doc,
"Prior noise rate (read/write)\n"
"\n");

PyDoc_STRVAR(
  VFR_getset_alpha_doc,
"Posterior noise shape (read-only)\n"
"\n");

PyDoc_STRVAR(
  VFR_getset_beta_doc,
"Posterior noise rate (read-only)\n"
"\n");

PyDoc_STRVAR(
  VFR_getset_tau_doc,
"Posterior noise precision (read-only)\n"
"\n");

/* VFR_bound(): return the lower bound of a vfr model.
 *  - see model_bound_fn() for more information.
 */
MODEL_BOUND (VFR) {
  /* initialize the computation. */
  double bound = 0.0;

  /* include the complexity term. */
  for (size_t k = 0; k < mdl->K; k++)
    bound -= log(matrix_get(mdl->L, k, k));

  /* include the data fit term. */
  bound -= mdl->alpha * log(mdl->beta);

  /* return the computed result. */
  return bound;
}

/* VFR_predict(): return the prediction of a vfr model.
 *  - see model_predict_fn() for more information.
 */
MODEL_PREDICT (VFR) {
  /* initialize the predicted mean. */
  double mu = 0.0;

  /* loop over the terms of the inner product. */
  for (size_t j = 0, i = 0; j < mdl->M; j++) {
    for (size_t k = 0; k < mdl->factors[j]->K; k++, i++) {
      /* include the current contribution from the inner product. */
      mu += vector_get(mdl->wbar, i) * model_mean(mdl, x, p, j, k);
    }
  }

  /* compute the expected noise variance and initialize
   * the predicted variance.
   */
  const double tauinv = mdl->beta / (mdl->alpha - 1.0);
  double eta = tauinv - mu * mu;

  /* loop over the first trace dimension. */
  for (size_t j1 = 0, i1 = 0; j1 < mdl->M; j1++) {
    for (size_t k1 = 0; k1 < mdl->factors[j1]->K; k1++, i1++) {
      /* loop over the second trace dimension. */
      for (size_t j2 = 0, i2 = 0; j2 < mdl->M; j2++) {
        for (size_t k2 = 0; k2 < mdl->factors[j2]->K; k2++, i2++) {
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

/* VFR_infer(): perform complete inference in a vfr model.
 *  - see model_infer_fn() for more information.
 */
MODEL_INFER (VFR) {
  /* gain access to the dataset structure members. */
  const size_t N = mdl->dat->N;
  Data *dat = mdl->dat;
  Datum *di;

  /* declare views into the weight precisions and the projections. */
  MatrixView G;
  VectorView h;

  /* loop over the factors. */
  for (size_t j1 = 0, i1 = 0; j1 < mdl->M; j1++) {
    /* get the projection subvector. */
    const size_t K1 = mdl->factors[j1]->K;
    h = vector_subvector(mdl->h, i1, K1);

    /* loop over the weights of the current factor. */
    for (size_t k1 = 0; k1 < K1; k1++) {
      /* initialize the projection subvector element. */
      double hk = 0.0;

      /* compute the contributions of each observation. */
      for (size_t i = 0; i < N; i++) {
        di = data_get(dat, i);
        hk += di->y * model_mean(mdl, di->x, di->p, j1, k1);
      }

      /* store the projection subvector element. */
      vector_set(&h, k1, hk);

      /* loop again over the factors. */
      for (size_t j2 = 0, i2 = 0; j2 < mdl->M; j2++) {
        /* get the precision submatrix. */
        const size_t K2 = mdl->factors[j2]->K;
        G = matrix_submatrix(mdl->Sinv, i1, i2, K1, K2);

        /* loop again over the factor weights. */
        for (size_t k2 = 0; k2 < K2; k2++) {
          /* initialize the precision submatrix element. */
          double gkk = 0.0;

          /* compute the contributions of each observation. */
          for (size_t i = 0; i < N; i++) {
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
  VectorView Gdiag = matrix_diag(mdl->Sinv);
  vector_add_const(&Gdiag, mdl->nu);

  /* compute the cholesky decomposition of the weight precisions. */
  matrix_copy(mdl->L, mdl->Sinv);
  chol_decomp(mdl->L);

  /* update the weight means and covariances. */
  chol_solve(mdl->L, mdl->h, mdl->wbar);
  chol_invert(mdl->L, mdl->Sigma);

  /* compute the model and data inner products. */
  VectorView z = vector_subvector(mdl->tmp, 0, mdl->K);
  blas_dtrmv(BLAS_TRANS, mdl->L, mdl->wbar, &z);
  const double wSw = blas_ddot(&z, &z);
  const double yy = data_inner(dat);

  /* update the noise shape and rate. */
  mdl->alpha = mdl->alpha0 + 0.5 * (double) N;
  mdl->beta = mdl->beta0 + 0.5 * (yy - wSw);

  /* update the noise precision. */
  mdl->tau = mdl->alpha / mdl->beta;

  /* return success. */
  return 1;
}

/* VFR_update(): perform efficient low-rank inference in a vfr model.
 *  - see model_update_fn() for more information.
 */
MODEL_UPDATE (VFR) {
  /* get the weight offset and count of the current factor. */
  const size_t k0 = model_weight_idx(mdl, j, 0);
  const size_t K = mdl->factors[j]->K;

  /* gain access to the dataset structure members. */
  const size_t N = mdl->dat->N;
  Data *dat = mdl->dat;
  Datum *di;

  /* prepare for low-rank adjustment. */
  model_weight_adjust_init(mdl, j);

  /* loop over the weights of the current factor. */
  for (size_t k = 0; k < K; k++) {
    /* initialize the projection subvector element. */
    double hk = 0.0;

    /* compute the contributions of each observation. */
    for (size_t i = 0; i < N; i++) {
      di = data_get(dat, i);
      hk += di->y * model_mean(mdl, di->x, di->p, j, k);
    }

    /* store the projection subvector element. */
    vector_set(mdl->h, k0 + k, hk);

    /* loop again over the factors. */
    for (size_t j2 = 0, i2 = 0; j2 < mdl->M; j2++) {
      /* get the precision submatrix. */
      const size_t K2 = mdl->factors[j2]->K;

      /* loop again over the factor weights. */
      for (size_t k2 = 0; k2 < K2; k2++) {
        /* initialize the precision submatrix element. */
        double gkk = 0.0;

        /* compute the contributions of each observation. */
        for (size_t i = 0; i < N; i++) {
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
  for (size_t k = 0; k < K; k++) {
    double gkk = matrix_get(mdl->Sinv, k0 + k, k0 + k);
    matrix_set(mdl->Sinv, k0 + k, k0 + k, gkk + mdl->nu);
  }

  /* perform low-rank adjustment. */
  if (!model_weight_adjust(mdl, j))
    return 0;

  /* update the weight means. */
  chol_solve(mdl->L, mdl->h, mdl->wbar);

  /* compute the model inner product. */
  VectorView z = vector_subvector(mdl->tmp, 0, mdl->K);
  blas_dtrmv(BLAS_TRANS, mdl->L, mdl->wbar, &z);
  const double wSw = blas_ddot(&z, &z);

  /* compute the data inner product. */
  double yy = 0.0;
  for (size_t i = 0; i < N; i++) {
    di = data_get(dat, i);
    yy += di->y * di->y;
  }

  /* update the noise shape and rate. */
  mdl->alpha = mdl->alpha0 + 0.5 * (double) N;
  mdl->beta = mdl->beta0 + 0.5 * (yy - wSw);

  /* update the noise precision. */
  mdl->tau = mdl->alpha / mdl->beta;

  /* if the update failed to yield reasonable values, perform
   * a full re-inference.
   */
  if (!isfinite(mdl->beta))
    return VFR_infer(mdl);

  /* return success. */
  return 1;
}

/* VFR_gradient(): return the gradient of a single factor in a vfr model.
 *  - see model_gradient_fn() for more information.
 */
MODEL_GRADIENT (VFR) {
  /* determine the weight index offset of the current factor. */
  const size_t k0 = model_weight_idx(mdl, j, 0);

  /* gain access to the factor weight count. */
  const Factor *fj = mdl->factors[j];
  const size_t K = fj->K;

  /* gain access to the specified observation. */
  Datum *di = data_get(mdl->dat, i);
  const size_t p = di->p;
  const Vector *x = di->x;
  const double y = di->y;

  /* gain access to the expected noise precision. */
  const double tau = mdl->tau;

  /* create the vector view for individual gradient terms. */
  VectorView g = vector_subvector(mdl->tmp, mdl->K, grad->len);

  /* loop over the weights of the current factor. */
  for (size_t k = 0; k < K; k++) {
    /* compute the weight first moment. */
    const double wk = vector_get(mdl->wbar, k0 + k);

    /* loop over the other weights of the current factor. */
    for (size_t kk = 0; kk < K; kk++) {
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
    for (size_t j2 = 0, i2 = 0; j2 < mdl->M; j2++) {
      /* get the weight count of the other factor. */
      const size_t K2 = mdl->factors[j2]->K;

      /* the diagonal second-order contribution was already included. */
      if (j2 == j) {
        i2 += K2;
        continue;
      }

      /* loop over the other factor weights. */
      for (size_t k2 = 0; k2 < K2; k2++) {
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

/* VFR_meanfield(): return the coefficients required for an
 * assumed-density mean-field update of a factor in a vfr model.
 *  - see model_meanfield_fn() for more information.
 */
MODEL_MEANFIELD (VFR) {
  /* get the weight offset and count of the current factor. */
  const size_t k0 = model_weight_idx(mdl, j, 0);
  const size_t K = mdl->factors[j]->K;

  /* gain access to the dataset structure members. */
  Datum *dat = mdl->dat->data + i;
  const size_t M = mdl->M;

  /* gain access to the expected noise precision. */
  const double tau = mdl->tau;

  /* create views into the factor weight means and covariances. */
  VectorView wk = vector_subvector(mdl->wbar, k0, K);
  MatrixView Sk = matrix_submatrix(mdl->Sigma, k0, k0, K, K);

  /* loop over the weights of the current factor. */
  for (size_t k = 0; k < K; k++) {
    /* compute and store the contribution. */
    const double bk = tau * dat->y * vector_get(&wk, k);
    vector_set(b, k, bk);
  }

  /* loop over the model factors. */
  for (size_t j2 = 0, i2 = 0; j2 < M; j2++) {
    /* get the number of other-factor weights. */
    const size_t K2 = mdl->factors[j2]->K;

    /* exclude the factor being updated. */
    if (j2 == j) {
      i2 += K2;
      continue;
    }

    /* loop over the weights of the other factor. */
    for (size_t k2 = 0; k2 < K2; k2++) {
      /* get the other factor mean. */
      const double phi2 = model_mean(mdl, dat->x, dat->p, j2, k2);

      /* loop over the weights of the current factor. */
      for (size_t k = 0; k < K; k++) {
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
  for (size_t k = 0; k < K; k++) {
    for (size_t k2 = 0; k2 < K; k2++) {
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

/* --- */

/* VFR_get_alpha0(): method to get model prior noise shape parameters.
 */
static PyObject*
VFR_get_alpha0 (Model *mdl) {
  /* return the shape parameter as a float. */
  return PyFloat_FromDouble(mdl->alpha0);
}

/* VFR_set_alpha0(): method to set model prior noise shape parameters.
 */
static int
VFR_set_alpha0 (Model *mdl, PyObject *value, void *closure) {
  /* get the new value. */
  const double a0 = PyFloat_AsDouble(value);
  if (PyErr_Occurred())
    return -1;

  /* set the shape. */
  if (!model_set_alpha0(mdl, a0)) {
    PyErr_SetString(PyExc_ValueError, "expected positive shape");
    return -1;
  }

  /* return success. */
  return 0;
}

/* VFR_get_beta(): method to get model prior noise rate parameters.
 */
static PyObject*
VFR_get_beta0 (Model *mdl) {
  /* return the rate parameter as a float. */
  return PyFloat_FromDouble(mdl->beta0);
}

/* VFR_set_beta0(): method to set model prior noise rate parameters.
 */
static int
VFR_set_beta0 (Model *mdl, PyObject *value, void *closure) {
  /* get the new value. */
  const double b0 = PyFloat_AsDouble(value);
  if (PyErr_Occurred())
    return -1;

  /* set the rate. */
  if (!model_set_beta0(mdl, b0)) {
    PyErr_SetString(PyExc_ValueError, "expected positive rate");
    return -1;
  }

  /* return success. */
  return 0;
}

/* VFR_get_alpha(): method to get model noise shape parameters.
 */
static PyObject*
VFR_get_alpha (Model *mdl) {
  /* return the shape parameter as a float. */
  return PyFloat_FromDouble(mdl->alpha);
}

/* VFR_get_beta(): method to get model noise rate parameters.
 */
static PyObject*
VFR_get_beta (Model *mdl) {
  /* return the rate parameter as a float. */
  return PyFloat_FromDouble(mdl->beta);
}

/* VFR_get_tau(): method to get model precision parameters.
 */
static PyObject*
VFR_get_tau (Model *mdl) {
  /* return the precision parameter as a float. */
  return PyFloat_FromDouble(mdl->tau);
}

/* --- */

/* VFR_new(): allocate a new vfr model.
 *  - see PyTypeObject.tp_new for details.
 */
VFL_TYPE_NEW (VFR) {
  /* allocate a new model. */
  VFR *self = (VFR*) type->tp_alloc(type, 0);
  Model_reset((Model*) self);
  if (!self)
    return NULL;

  /* set the function pointers. */
  Model *mdl = (Model*) self;
  mdl->bound     = VFR_bound;
  mdl->predict   = VFR_predict;
  mdl->infer     = VFR_infer;
  mdl->update    = VFR_update;
  mdl->gradient  = VFR_gradient;
  mdl->meanfield = VFR_meanfield;

  /* return the new object. */
  return (PyObject*) self;
}

/* VFR_getset: property definition structure for vfr models.
 */
static PyGetSetDef VFR_getset[] = {
  { "alpha0",
    (getter) VFR_get_alpha0,
    (setter) VFR_set_alpha0,
    VFR_getset_alpha0_doc,
    NULL
  },
  { "beta0",
    (getter) VFR_get_beta0,
    (setter) VFR_set_beta0,
    VFR_getset_beta0_doc,
    NULL
  },
  { "alpha",
    (getter) VFR_get_alpha,
    NULL,
    VFR_getset_alpha_doc,
    NULL
  },
  { "beta",
    (getter) VFR_get_beta,
    NULL,
    VFR_getset_beta_doc,
    NULL
  },
  { "tau",
    (getter) VFR_get_tau,
    NULL,
    VFR_getset_tau_doc,
    NULL
  },
  { NULL }
};

/* VFR_methods: method definition structure for vfr models.
 */
static PyMethodDef VFR_methods[] = {
  { NULL }
};

/* VFR_Type, VFR_Type_init() */
VFL_TYPE (VFR, Model, model)

