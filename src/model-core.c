
/* include the vfl header. */
#include <vfl/vfl.h>

/* model_kmax(): determine the maximum weight count from an array
 * of factors.
 *
 * arguments:
 *  @fv: array of factors, or null.
 *  @M: size of the array of factors.
 *
 * returns:
 *  maximum weight count. if @fv is null, the count is zero.
 *  if any fv[j] is null, its weight count is treated as zero.
 */
static inline size_t
model_kmax (Factor **fv, size_t M) {
  /* find the largest weight count from all non-null factors. */
  size_t kmax = 0;
  for (size_t j = 0; j < M; j++)
    kmax = (fv && fv[j] && fv[j]->K > kmax ? fv[j]->K : kmax);

  /* return the identified value. */
  return kmax;
}

/* model_tmp(): determine the number of temporary scalars required
 * by a variational feature model with a certain set of sizes.
 *
 * arguments:
 *  @fv: array of factors to use for weight counting.
 *  @P, @M, @K: new sizes to use for the computation.
 *
 * returns:
 *  required number of temporary scalars.
 */
static inline size_t
model_tmp (Factor **fv, size_t P, size_t M, size_t K) {
  /* in order to conserve memory, determine the largest number of
   * factor weights that will be updated at the same time.
   */
  const size_t kmax = model_kmax(fv, M);

  /* the scalars are laid out as follows:
   *  z: (K, 1)         | b: (max(k), 1)
   *  U: (max(k), K)    | B: (max(k), max(k))
   *  V: (max(k), K)    |
   */
  const size_t ntmp = K + P + 2 * kmax * K;

  /* return the computed scalar count. */
  return ntmp;
}

/* model_internal_refresh(): refresh the internal state of a model.
 *
 * arguments:
 *  @mdl: model structure pointer to access.
 *
 * returns:
 *  integer indicating whether (1) or not (0) the refresh succeeded.
 */
static inline int
model_internal_refresh (Model *mdl, size_t D, size_t P,
                                    size_t M, size_t K) {
  /* allocate new vectors. */
  Vector *wbar = vector_alloc(K);
  Vector *h = vector_alloc(K);
  Vector *tmp = NULL; /* will be allocated later. */

  /* allocate new matrices. */
  Matrix *Sigma = matrix_alloc(K, K);
  Matrix *Sinv = matrix_alloc(K, K);
  Matrix *L = matrix_alloc(K, K);

  /* if the factor count changed, allocate new factor arrays. */
  Factor **factors = NULL;
  if (M != mdl->M)
    factors = malloc(2 * M * sizeof(Factor*));
  else
    factors = mdl->factors;

  /* check if any single allocation failed. */
  if (!wbar || !h || !Sigma || !Sinv || !L || !factors)
    goto fail;

  /* get the prior factor array. */
  Factor **priors = factors + M;

  /* if the factor count changed, initialize the contents
   * of the new factor array.
   */
  if (factors != mdl->factors) {
    /* initialize the new array contents. */
    for (size_t j = 0; j < M; j++) {
      if (j < mdl->M) {
        /* copy factors that will remain from the old array. */
        factors[j] = mdl->factors[j];
        priors[j] = mdl->priors[j];
      }
      else {
        /* initialize extra factors to null. */
        factors[j] = NULL;
        priors[j] = NULL;
      }
    }
  }

  /* allocate the temporary array. last chance to fail. */
  tmp = vector_alloc(model_tmp(factors, P, M, K));
  if (!tmp)
    goto fail;

  /* if the factor count changed, free the old factor array. */
  if (factors != mdl->factors) {
    /* drop references to all lost factors. */
    for (size_t j = M; j < mdl->M; j++) {
      Py_DECREF(mdl->factors[j]);
      Py_DECREF(mdl->priors[j]);
    }

    /* free the old factor array. */
    free(mdl->factors);
  }

  /* free the model vectors. */
  vector_free(mdl->wbar);
  vector_free(mdl->h);
  vector_free(mdl->tmp);

  /* free the model matrices. */
  matrix_free(mdl->Sigma);
  matrix_free(mdl->Sinv);
  matrix_free(mdl->L);

  /* set the new factor array. */
  mdl->factors = factors;
  mdl->priors = priors;

  /* set the new vectors. */
  mdl->wbar = wbar;
  mdl->h = h;
  mdl->tmp = tmp;

  /* set the new matrices. */
  mdl->Sigma = Sigma;
  mdl->Sinv = Sinv;
  mdl->L = L;

  /* store the new sizes into the model. */
  mdl->D = D;
  mdl->P = P;
  mdl->M = M;
  mdl->K = K;

  /* return success. */
  return 1;

fail:
  /* free the vectors. */
  vector_free(wbar);
  vector_free(h);
  vector_free(tmp);

  /* free the matrices. */
  matrix_free(Sigma);
  matrix_free(Sinv);
  matrix_free(L);

  /* free the factors, if they were allocated. */
  if (factors != mdl->factors)
    free(factors);

  /* return failure. */
  return 0;
}

/* --- */

/* Model_reset(): reset the contents of a model structure.
 *
 * arguments:
 *  @mdl: model structure pointer to modify.
 */
void Model_reset (Model *mdl) {
  /* return if the struct pointer is null. */
  if (!mdl)
    return;

  /* initialize the function pointers. */
  mdl->init      = NULL;
  mdl->bound     = NULL;
  mdl->predict   = NULL;
  mdl->infer     = NULL;
  mdl->update    = NULL;
  mdl->gradient  = NULL;
  mdl->meanfield = NULL;

  /* initialize the sizes. */
  mdl->D = 0;
  mdl->P = 0;
  mdl->M = 0;
  mdl->K = 0;

  /* initialize the prior parameters. */
  mdl->alpha0 = 1.0;
  mdl->beta0 = 1.0;
  mdl->nu = 1.0;

  /* initialize the posterior noise parameters. */
  mdl->alpha = mdl->alpha0;
  mdl->beta = mdl->beta0;
  mdl->tau = mdl->alpha / mdl->beta;

  /* initialize the posterior weight parameters. */
  mdl->wbar = NULL;
  mdl->Sigma = NULL;

  /* initialize the posterior logistic parameters. */
  mdl->xi = NULL;

  /* initialize the intermediates. */
  mdl->Sinv = NULL;
  mdl->L = NULL;
  mdl->h = NULL;

  /* initialize the prior and posterior factor arrays. */
  mdl->factors = NULL;
  mdl->priors = NULL;

  /* initialize the associated dataset. */
  mdl->dat = NULL;

  /* initialize the temporary vector. */
  mdl->tmp = NULL;
}

/* model_set_alpha0(): set the noise precision shape-prior of a model.
 *
 * arguments:
 *  @mdl: model structure pointer to modify.
 *  @alpha0: new parameter value.
 *
 * returns:
 *  integer indicating assignment success (1) or failure (0).
 */
int model_set_alpha0 (Model *mdl, double alpha0) {
  /* check that the model is valid and the parameter is in bounds. */
  if (!mdl || alpha0 <= 0.0)
    return 0;

  /* store the parameter and return success. */
  mdl->alpha = mdl->alpha0 = alpha0;
  mdl->tau = mdl->alpha / mdl->beta;
  return 1;
}

/* model_set_beta0(): set the noise precision rate-prior of a model.
 *
 * arguments:
 *  @mdl: model structure pointer to modify.
 *  @beta0: new parameter value.
 *
 * returns:
 *  integer indicating assignment success (1) or failure (0).
 */
int model_set_beta0 (Model *mdl, double beta0) {
  /* check that the model is valid and the parameter is in bounds. */
  if (!mdl || beta0 <= 0.0)
    return 0;

  /* store the parameter and return success. */
  mdl->beta = mdl->beta0 = beta0;
  mdl->tau = mdl->alpha / mdl->beta;
  return 1;
}

/* model_set_nu(): set the weight relative precision-prior of a model.
 *
 * arguments:
 *  @mdl: model structure pointer to modify.
 *  @nu: new parameter value.
 *
 * returns:
 *  integer indicating assignment success (1) or failure (0).
 */
int model_set_nu (Model *mdl, double nu) {
  /* check that the model is valid and the parameter is in bounds. */
  if (!mdl || nu <= 0.0)
    return 0;

  /* store the parameter and return success. */
  mdl->nu = nu;
  return 1;
}

/* model_set_parms(): set the parameter vector of a model factor.
 *
 * arguments:
 *  @mdl: model structure pointer to modify.
 *  @j: index of the factor to modify.
 *  @par: new parameter vector to set.
 *
 * returns:
 *  integer indicating whether (1) or not (0) assignment was successful.
 *  this check includes the bounds-check for each factor parameter.
 */
int model_set_parms (Model *mdl, size_t j, const Vector *par) {
  /* check the pointers, indices and sizes. */
  if (!mdl || !par || j >= mdl->M || par->len != mdl->factors[j]->P)
    return 0;

  /* loop over each parameter. */
  for (size_t p = 0; p < par->len; p++) {
    /* return failure if any single parameter update fails. */
    if (!factor_set(mdl->factors[j], p, vector_get(par, p)))
      return 0;
  }

  /* return success. */
  return 1;
}

/* model_set_data(): associate a dataset with a variational feature model.
 * this will free the currently associated dataset.
 *
 * arguments:
 *  @mdl: model structure pointer to modify.
 *  @dat: dataset structure pointer to assign.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int model_set_data (Model *mdl, Data *dat) {
  /* check the structure pointers. */
  if (!mdl || !dat)
    return 0;

  /* if the dataset and model are non-empty, make
   * sure they are compatible with each other.
   */
  if (mdl->D && dat->N && dat->D < mdl->D)
    return 0;

  /* allocate new logistic parameters and temporary coefficients. */
  const size_t ntmp = model_tmp(mdl->factors, mdl->P, mdl->M, mdl->K);
  Vector *tmp = vector_alloc(ntmp);
  Vector *xi = vector_alloc(dat->N);
  if (!tmp || !xi)
    return 0;

  /* replace the logistic parameters and temporary coefficients. */
  vector_free(mdl->tmp);
  vector_free(mdl->xi);
  mdl->tmp = tmp;
  mdl->xi = xi;

  /* initialize the logistic parameters. */
  vector_set_all(mdl->xi, 1.0);

  /* store the new dataset. */
  Py_XDECREF(mdl->dat);
  Py_INCREF(dat);
  mdl->dat = dat;

  /* return succes. */
  return 1;
}

/* model_set_factor(): replace a variational feature/factor in a model.
 *
 * arguments:
 *  @mdl: model structure pointer to modify.
 *  @i: factor array index to modify.
 *  @f: new variational factor.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int model_set_factor (Model *mdl, size_t i, Factor *f) {
  /* check the input pointers. */
  if (!mdl || !f)
    return 0;

  /* check the factor index. */
  if (i >= mdl->M)
    return 0;

  /* get the (old) replaced factor and its prior. */
  Factor *fi = mdl->factors[i];
  Factor *pi = mdl->priors[i];

  /* determine the new sizes of the model. */
  const size_t P = mdl->P - fi->P + f->P;
  const size_t K = mdl->K - fi->K + f->K;
  const size_t M = mdl->M;

  /* determine the new dimensionality of the model. */
  size_t D = 0;
  for (size_t j = 0; j < M; j++) {
    if (j == i)
      D = (D > f->D ? D : f->D);
    else
      D = (D > mdl->factors[j]->D ? D : mdl->factors[j]->D);
  }

  /* update the factor-dependent model internals. */
  if (!model_internal_refresh(mdl, D, P, M, K))
    return 0;

  /* create a prior copy of the factor. */
  Factor *p = factor_copy(f);
  if (!p)
    return 0;

  /* set the new factor and prior. */
  Py_DECREF(fi);
  Py_DECREF(pi);
  Py_INCREF(f);
  mdl->factors[i] = f;
  mdl->priors[i] = p;

  /* return success. */
  return 1;
}

/* model_add_factor(): incorporate a new variational feature/factor
 * into a model.
 *
 * arguments:
 *  @mdl: model structure pointer to modify.
 *  @f: variational factor to add.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int model_add_factor (Model *mdl, Factor *f) {
  /* check the input pointers. */
  if (!mdl || !f)
    return 0;

  /* determine the new sizes of the model. */
  const size_t D = (mdl->D > f->D ? mdl->D : f->D);
  const size_t P = mdl->P + f->P;
  const size_t K = mdl->K + f->K;
  const size_t M = mdl->M + 1;

  /* update the factor-dependent model internals. */
  if (!model_internal_refresh(mdl, D, P, M, K))
    return 0;

  /* create a prior copy of the factor. */
  Factor *p = factor_copy(f);
  if (!p)
    return 0;

  /* store the factor and its prior. */
  Py_INCREF(f);
  mdl->factors[M - 1] = f;
  mdl->priors[M - 1] = p;

  /* return success. */
  return 1;
}

/* model_clear_factors(): remove all factors from a model.
 *
 * arguments:
 *  @mdl: model structure pointer to modify.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int model_clear_factors (Model *mdl) {
  /* check the input pointer. */
  if (!mdl)
    return 0;

  /* if the factor list is already empty, return. */
  if (mdl->M == 0)
    return 1;

  /* return the result of zeroing the factor count. */
  return model_internal_refresh(mdl, 0, 0, 0, 0);
}

/* model_mean(): return the first moment of a model basis element.
 *
 * arguments:
 *  @mdl: model structure pointer.
 *  @x: observation input vector.
 *  @p: function output index.
 *  @j: factor index.
 *  @k: basis index.
 *
 * returns:
 *  expectation of the requested basis element.
 */
double model_mean (const Model *mdl, const Vector *x,
                   size_t p, size_t j, size_t k) {
  /* check the input pointers and indices. */
  if (!mdl || !x || j >= mdl->M || k >= mdl->factors[j]->K)
    return 0.0;

  /* return the requested expectation. */
  return factor_mean(mdl->factors[j], x, p, k);
}

/* model_var(): return the second moment of a pair of
 * model basis elements.
 *
 * arguments:
 *  @mdl: model structure pointer.
 *  @x: observation input vector.
 *  @p: function output index.
 *  @j1: first factor index.
 *  @j2: second factor index.
 *  @k1: first basis index.
 *  @k2: second basis index.
 *
 * returns:
 *  expectation of the requested product of basis elements.
 */
double model_var (const Model *mdl, const Vector *x,
                  size_t p, size_t j1, size_t j2, size_t k1, size_t k2) {
  /* check the input pointers and indices. */
  if (!mdl || !x || j1 >= mdl->M || j2 >= mdl->M ||
      k1 >= mdl->factors[j1]->K ||
      k2 >= mdl->factors[j2]->K)
    return 0.0;

  /* if the factor indices are different, then the expectation
   * of the product simplifies to the product of expectations.
   */
  if (j1 != j2)
    return factor_mean(mdl->factors[j1], x, p, k1) *
           factor_mean(mdl->factors[j2], x, p, k2);

  /* return the requested expectation. */
  return factor_var(mdl->factors[j1], x, p, k1, k2);
}

/* model_cov(): return the covariance of a model function at
 * two input locations.
 *
 * arguments:
 *  @mdl: model structure pointer.
 *  @x1: first observation input vector.
 *  @x2: second observation input vector.
 *  @p1: first function output index.
 *  @p2: second function output index.
 *
 * returns:
 *  expectation of the requested product of basis elements.
 */
double model_cov (const Model *mdl, const Vector *x1, const Vector *x2,
                  size_t p1, size_t p2) {
  /* check the input pointers. */
  if (!mdl || !x1 || !x2)
    return 0.0;

  /* initialize the computation. */
  double cov = 0.0;

  /* sum together the contributions from each factor. */
  for (size_t j = 0; j < mdl->M; j++)
    cov += factor_cov(mdl->factors[j], x1, x2, p1, p2);

  /* return the computed result. */
  return (cov / mdl->nu + (double) vector_equal(x1, x2)) / mdl->tau;
}

/* model_kernel(): write the covariance kernel function code
 * of a variational feature model.
 *
 * arguments:
 *  @mdl: model structure pointer to access.
 *
 * returns:
 *  newly allocated string that contains the covariance kernel
 *  function code required to evaluate model covariances in
 *  opencl.
 */
char *model_kernel (const Model *mdl) {
  /* check the input pointer. */
  if (!mdl)
    return NULL;

  /* define the kernel code format string. */
  const char *fmt = "{\n%s}\nsum += cov;\n";

  /* allocate an array for storing factor kernel code. */
  char **fstr = malloc(mdl->M * sizeof(char*));
  if (!fstr)
    return NULL;

  /* get the strings of each factor. */
  for (size_t j = 0, pj = 1; j < mdl->M; j++) {
    /* get the current factor string. */
    const Factor *fj = mdl->factors[j];
    fstr[j] = factor_kernel(fj, pj);

    /* check for failure. */
    if (!fstr[j])
      return NULL;

    /* advance the factor parameter offset. */
    pj += fj->P;
  }

  /* determine the length of the kernel code string. */
  size_t len = 8;
  for (size_t j = 0; j < mdl->M; j++)
    len += strlen(fmt) + strlen(fstr[j]);

  /* allocate the kernel code string. */
  char *kstr = malloc(len);
  if (!kstr)
    return NULL;

  /* write each factor string. */
  char *pos = kstr;
  for (size_t j = 0; j < mdl->M; j++) {
    pos += sprintf(pos, fmt, fstr[j]);
    free(fstr[j]);
  }

  /* free the factor string array. */
  free(fstr);

  /* return the new string. */
  return kstr;
}

/* model_bound(): return the model variational lower bound.
 *  - see model_bound_fn() for more information.
 */
double model_bound (const Model *mdl) {
  /* check the input pointer. */
  if (!mdl)
    return 0.0;

  /* check the function pointer. */
  if (!mdl->bound)
    return 0.0;

  /* initialize a variable to hold the divergence penalty. */
  double div = 0.0;

  /* include the divergences of each factor into the penalty. */
  for (size_t j = 0; j < mdl->M; j++)
    div += factor_div(mdl->factors[j], mdl->priors[j]);

  /* execute the assigned bound function and
   * include the divergence penalty.
   */
  return mdl->bound(mdl) - div;
}

/* model_eval(): evalute the underlying linear function of a model
 * at the current mode of its variational distribution.
 *
 * arguments:
 *  @mdl: model structure pointer.
 *  @x: observation input vector.
 *  @p: function output index.
 *
 * returns:
 *  maximum posterior estimate of the model function.
 */
double model_eval (const Model *mdl, const Vector *x, size_t p) {
  /* check the input pointers. */
  if (!mdl || !x)
    return 0.0;

  /* compute the model estimate. */
  double mode = 0.0;
  for (size_t j = 0, i = 0; j < mdl->M; j++) {
    /* get the current factor. */
    const Factor *fj = mdl->factors[j];

    /* loop over the weights assigned to the factor. */
    for (size_t k = 0; k < fj->K; k++, i++)
      mode += vector_get(mdl->wbar, i) * factor_eval(fj, x, p, k);
  }

  /* return computed estimate. */
  return mode;
}

/* model_predict(): return the model posterior prediction.
 *  - see model_predict_fn() for more information.
 */
int model_predict (const Model *mdl, const Vector *x, size_t p,
                   double *mean, double *var) {
  /* check the input pointers. */
  if (!mdl || !x || !mean || !var)
    return 0;

  /* check the function pointer. */
  if (!mdl->predict)
    return 0;

  /* check the vector size. */
  if (x->len < mdl->D)
    return 0;

  /* execute the assigned prediction function. */
  return mdl->predict(mdl, x, p, mean, var);
}

/* model_eval_all(): return model maximum posterior evaluations
 * for all observations in a dataset.
 *
 * arguments:
 *  @mdl: model structure pointer.
 *  @dat: dataset for evaluation storage.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int model_eval_all (const Model *mdl, Data *dat) {
  /* check the input pointers. */
  if (!mdl || !dat)
    return 0;

  /* check that the model and input dataset match in dimensionality. */
  if (dat->D != mdl->D)
    return 0;

  /* loop over each observation. */
  for (size_t i = 0; i < dat->N; i++) {
    /* compute and store the model evaluation. */
    Datum *di = data_get(dat, i);
    di->y = model_eval(mdl, di->x, di->p);
  }

  /* return success. */
  return 1;
}

/* model_predict_all(): return model posterior predictions for
 * all observations in a pair of datasets. the two datasets
 * must have equal sizes, but no checking is performed on
 * their observations.
 *
 * arguments:
 *  @mdl: model structure pointer.
 *  @mean: dataset for predicted mean storage.
 *  @var: dataset for predicted variance storage.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int model_predict_all (const Model *mdl, Data *mean, Data *var) {
  /* declare required variables:
   *  @mu, @eta: individual means and variances.
   *  @xdata: dataset used for predictions.
   */
  double mu, eta;
  Data *xdata;

  /* check the input pointers. */
  if (!mdl)
    return 0;

  /* determine which dataset to use for input locations. */
  if (mean)
    xdata = mean;
  else if (var)
    xdata = var;
  else
    return 0;

  /* if both datasets are provided, make sure they have matching size. */
  if (mean && var && (mean->D != var->D || mean->N != var->N))
    return 0;

  /* check that the model and input dataset match in dimensionality. */
  if (xdata->D != mdl->D)
    return 0;

  /* loop over each observation. */
  for (size_t i = 0; i < xdata->N; i++) {
    /* compute the posterior mean and variance. */
    Datum *xdatum = data_get(xdata, i);
    model_predict(mdl, xdatum->x, xdatum->p, &mu, &eta);

    /* store the predictions. */
    if (mean) mean->data[i].y = mu;
    if (var)  var->data[i].y = eta;
  }

  /* return success. */
  return 1;
}

/* model_reset(): reset the factor parameters of a model to those
 * of their respective prior factors.
 *
 * arguments:
 *  @mdl: model structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int model_reset (Model *mdl) {
  /* check the input pointer. */
  if (!mdl)
    return 0;

  /* loop over the factors, copying parameters from the priors. */
  for (size_t j = 0; j < mdl->M; j++) {
    /* set the factor parameters from the prior. */
    const Factor *fp = mdl->priors[j];
    if (!model_set_parms(mdl, j, fp->par))
      return 0;
  }

  /* return success. */
  return model_infer(mdl);
}

/* model_infer(): fully update the nuisance parameters of a model.
 *  - see model_infer_fn() for more information.
 */
int model_infer (Model *mdl) {
  /* check the input pointer. */
  if (!mdl)
    return 0;

  /* check the function pointer. */
  if (!mdl->infer)
    return 0;

  /* execute the assigned inference function. */
  return mdl->infer(mdl);
}

/* model_update(): efficiently update the nuisance parameters of a model.
 *  - see model_update_fn() for more information.
 */
int model_update (Model *mdl, size_t j) {
  /* check the input pointer and factor index. */
  if (!mdl || j >= mdl->M)
    return 0;

  /* if an update function is assigned, execute it. */
  if (mdl->update && mdl->update(mdl, j))
    return 1;

  /* fall back to the infer function, if assigned. */
  if (mdl->infer)
    return mdl->infer(mdl);

  /* neither function is assigned. fail. */
  return 0;
}

/* model_gradient(): return the gradient of the lower bound.
 *  - see model_gradient_fn() for more information.
 */
int model_gradient (const Model *mdl, size_t i, size_t j, Vector *grad) {
  /* check the input pointers and factor index. */
  if (!mdl || !mdl->dat || !grad || i >= mdl->dat->N || j >= mdl->M)
    return 0;

  /* check if the factor has no parameters. */
  if (mdl->factors[j]->P == 0)
    return 1;

  /* check the gradient size. */
  if (grad->len != mdl->factors[j]->P)
    return 0;

  /* check the function pointer. */
  if (!mdl->gradient)
    return 0;

  /* execute the assigned gradient function. */
  return mdl->gradient(mdl, i, j, grad);
}

/* model_meanfield(): perform an assumed-density mean-field factor update.
 *
 * arguments:
 *  @mdl: model structure pointer.
 *  @j: index of the factor to update.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int model_meanfield (const Model *mdl, size_t j) {
  /* check the input pointers and factor index. */
  if (!mdl || !mdl->dat || j >= mdl->M)
    return 0;

  /* gain access to the factor and its number of weights and parameters. */
  Factor *f = mdl->factors[j];
  const size_t K = f->K;
  const size_t P = f->P;

  /* check if the factor has no parameters. */
  if (P == 0)
    return 1;

  /* check the model and factor function pointers. */
  if (!mdl->meanfield || !f->meanfield)
    return 0;

  /* gain access to the associated prior. */
  const Factor *fp = mdl->priors[j];

  /* create sets of coefficients for mean-field updates. */
  VectorView b = vector_view_array(mdl->tmp->data, K);
  MatrixView B = matrix_view_array(mdl->tmp->data + K, K, K);

  /* initialize the factor update. */
  if (!f->meanfield(f, NULL, NULL, NULL, NULL))
    return 0;

  /* loop over each data point. */
  Datum *di = mdl->dat->data;
  const size_t N = mdl->dat->N;
  for (size_t i = 0; i < N; i++, di++) {
    /* 1. compute the coefficients of the data point.
     * 2. stream the data point and coefficients to the factor.
     */
    mdl->meanfield(mdl, i, j, &b, &B);
    f->meanfield(f, fp, di, &b, &B);
  }

  /* finalize the factor update. */
  return f->meanfield(f, fp, NULL, NULL, NULL);
}

/* --- */

/* model_weight_idx(): return the index of a model weight that corresponds
 * to a specified basis index of a specified factor.
 *
 * arguments:
 *  @mdl: model structure pointer to access.
 *  @j: model factor index.
 *  @k: factor basis index.
 *
 * returns:
 *  corresponding model weight index.
 */
size_t model_weight_idx (const Model *mdl, size_t j, size_t k) {
  /* check the input pointers and indices. */
  if (!mdl || j >= mdl->M || k >= mdl->factors[j]->K)
    return 0;

  /* compute the weight offset for the specified factor. */
  size_t idx = 0;
  for (size_t j2 = 0; j2 < j; j2++)
    idx += mdl->factors[j2]->K;

  /* return the weight index. */
  return idx + k;
}

/* model_weight_adjust_init(): initialize data structures for performing
 * a new low-rank adjustment of the:
 *   a.) precision matrix cholesky factors.
 *   b.) covariance matrix.
 *
 * arguments:
 *  @mdl: model structure pointer to access.
 *  @j: model factor index.
 */
void model_weight_adjust_init (const Model *mdl, size_t j) {
  /* get the weight offset and count of the current factor. */
  const size_t k0 = model_weight_idx(mdl, j, 0);
  const size_t K = mdl->factors[j]->K;

  /* declare vector views for tracking precision matrix modifications. */
  VectorView u;
  MatrixView U;

  /* create the matrix view for storing updates. */
  U = matrix_view_array(mdl->tmp->data + mdl->K + mdl->P, K, mdl->K);

  /* copy the initial rows of the precision matrix. */
  for (size_t k = 0; k < K; k++) {
    u = matrix_row(&U, k);
    matrix_copy_row(&u, mdl->Sinv, k0 + k);
  }
}

/* model_weight_adjust_init(): perform a low-rank adjustment of the model
 * precision cholesky factors and covariances.
 *
 * arguments:
 *  @mdl: model structure pointer to access.
 *  @j: model factor index.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int model_weight_adjust (Model *mdl, size_t j) {
  /* get the weight offset and count of the current factor. */
  const size_t k0 = model_weight_idx(mdl, j, 0);
  const size_t K = mdl->factors[j]->K;

  /* declare vector views for tracking precision matrix modifications. */
  VectorView u, v, z;
  MatrixView U, V;

  /* create the vector view for holding individual rows/columns. */
  double *ptr = mdl->tmp->data;
  z = vector_view_array(ptr, mdl->K);
  ptr += mdl->K + mdl->P;

  /* create the matrix view for storing updates. */
  U = matrix_view_array(ptr, K, mdl->K);
  ptr += K * mdl->K;

  /* create the matrix view for storing downdates. */
  V = matrix_view_array(ptr, K, mdl->K);

  /* copy the final rows of the precision matrix. */
  for (size_t k = 0; k < K; k++) {
    v = matrix_row(&V, k);
    matrix_copy_row(&v, mdl->Sinv, k0 + k);
  }

  /* compute the difference between the precision matrix rows. */
  matrix_sub(&V, &U);

  /* compute the magnitude of the differences. */
  double vss = 0.0;
  for (size_t k = 0; k < K; k++) {
    v = matrix_row(&V, k);
    vss += blas_ddot(&v, &v);
  }

  /* fail if the differences have zero effective magnitude. */
  if (vss == 0.0)
    return 0;

  /* adjust the row differences for use in rank-1 updates. */
  for (size_t k = 0; k < K; k++) {
    /* scale the main diagonal element by one-half, and zero
     * all off-diagonals that have already been updated.
     */
    v = matrix_row(&V, k);
    vector_set(&v, k0 + k, 0.5 * vector_get(&v, k0 + k));
    for (size_t kk = 0; kk < k; kk++)
      vector_set(&v, k0 + kk, 0.0);
  }

  /* transform the row differences into symmetric updates and downdates. */
  for (size_t k = 0; k < K; k++) {
    /* get views of the update and downdate row vectors. */
    u = matrix_row(&U, k);
    v = matrix_row(&V, k);

    /* compute the symmetrization constants. */
    const double vnrm = blas_dnrm2(&v);
    const double alpha = sqrt(vnrm / 2.0);
    const double beta = 1.0 / vnrm;

    /* symmetrize the vectors. */
    for (size_t i = 0; i < mdl->K; i++) {
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
  for (size_t k = 0; k < K; k++) {
    /* update the cholesky factors. */
    u = matrix_row(&U, k);
    matrix_copy_row(&z, &U, k);
    chol_update(mdl->L, &z);

    /* update the covariance matrix. */
    blas_dgemv(BLAS_NO_TRANS, 1.0, mdl->Sigma, &u, 0.0, &z);
    double zudot = blas_ddot(&z, &u);
    zudot = 1.0 / (1.0 + zudot);
    for (size_t i = 0; i < mdl->K; i++)
      for (size_t j = 0; j < mdl->K; j++)
        matrix_set(mdl->Sigma, i, j,
          matrix_get(mdl->Sigma, i, j) -
          zudot * vector_get(&z, i) *
                  vector_get(&z, j));
  }

  /* apply the downdates. */
  for (size_t k = 0; k < K; k++) {
    /* downdate the cholesky factors. */
    v = matrix_row(&V, k);
    matrix_copy_row(&z, &V, k);
    chol_downdate(mdl->L, &z);

    /* downdate the covariance matrix. */
    blas_dgemv(BLAS_NO_TRANS, 1.0, mdl->Sigma, &v, 0.0, &z);
    double zvdot = blas_ddot(&z, &v);
    zvdot = 1.0 / (1.0 - zvdot);
    for (size_t i = 0; i < mdl->K; i++)
      for (size_t j = 0; j < mdl->K; j++)
        matrix_set(mdl->Sigma, i, j,
          matrix_get(mdl->Sigma, i, j) +
          zvdot * vector_get(&z, i) *
                  vector_get(&z, j));
  }

  /* return success. */
  return 1;
}

