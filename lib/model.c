
/* include the model header. */
#include <vfl/model.h>

/* model_tmp(): determine the number of temporary scalars required
 * by a variational feature model with a certain set of sizes.
 *
 * arguments:
 *  @mdl: model structure pointer to access.
 *
 * returns:
 *  required number of temporary scalars.
 */
static inline unsigned int model_tmp (const model_t *mdl) {
  /* declare required variables:
   *  @ntmp: computed number of scalars.
   *  @kmax: largest number of per-factor weights.
   */
  unsigned int ntmp, kmax;

  /* gain access to the parameter, factor, and weight counts.
   */
  const unsigned int P = mdl->P;
  const unsigned int M = mdl->M;
  const unsigned int K = mdl->K;

  /* in order to conserve memory, determine the largest number of
   * factor weights that will be updated at the same time.
   */
  kmax = 0;
  for (unsigned int j = 0; j < M; j++)
    kmax = (mdl->factors[j]->K > kmax ? mdl->factors[j]->K : kmax);

  /* the scalars are laid out as follows:
   *  z: (K, 1)         | b: (max(k), 1)
   *  U: (max(k), K)    | B: (max(k), max(k))
   *  V: (max(k), K)    |
   */
  ntmp = K + P + 2 * kmax * K;

  /* return the computed scalar count. */
  return ntmp;
}

/* model_alloc(): allocate a new empty variational feature model.
 *
 * arguments:
 *  @type: pointer to a model type structure.
 *
 * returns:
 *  newly allocated and initialized model structure pointer.
 */
model_t *model_alloc (const model_type_t *type) {
  /* check that the type structure is valid. */
  if (!type)
    return NULL;

  /* allocate the structure pointer. */
  model_t *mdl = malloc(type->size);
  if (!mdl)
    return NULL;

  /* initialize the model type. */
  mdl->type = *type;

  /* initialize the sizes of the model. */
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

  /* execute the initialization function, if defined. */
  model_init_fn init_fn = MODEL_TYPE(mdl)->init;
  if (init_fn && !init_fn(mdl)) {
    /* initialization failed. */
    model_free(mdl);
    return NULL;
  }

  /* return the new model. */
  return mdl;
}

/* model_free(): free an allocated model.
 *
 * arguments:
 *  @mdl: model structure pointer to free.
 */
void model_free (model_t *mdl) {
  /* return if the structure pointer is null. */
  if (!mdl)
    return;

  /* free the weight means and covariances. */
  vector_free(mdl->wbar);
  matrix_free(mdl->Sigma);

  /* free the logistic parameters. */
  vector_free(mdl->xi);

  /* free the intermediates. */
  matrix_free(mdl->Sinv);
  matrix_free(mdl->L);
  vector_free(mdl->h);

  /* free the individual prior and posterior factors. */
  for (unsigned int i = 0; i < mdl->M; i++) {
    factor_free(mdl->factors[i]);
    factor_free(mdl->priors[i]);
  }

  /* free the factor arrays. */
  free(mdl->factors);
  free(mdl->priors);

  /* free the temporary vector. */
  vector_free(mdl->tmp);

  /* free the structure pointer. */
  free(mdl);
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
int model_set_alpha0 (model_t *mdl, const double alpha0) {
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
int model_set_beta0 (model_t *mdl, const double beta0) {
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
int model_set_nu (model_t *mdl, const double nu) {
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
int model_set_parms (model_t *mdl, const unsigned int j,
                     const vector_t *par) {
  /* check the pointers, indices and sizes. */
  if (!mdl || !par || j >= mdl->M || par->len != mdl->factors[j]->P)
    return 0;

  /* loop over each parameter. */
  for (unsigned int p = 0; p < par->len; p++) {
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
int model_set_data (model_t *mdl, data_t *dat) {
  /* check the structure pointers. */
  if (!mdl || !dat)
    return 0;

  /* if the dataset and model are non-empty, make
   * sure they are compatible with each other.
   */
  if (mdl->D && dat->N && dat->D < mdl->D)
    return 0;

  /* drop the current dataset. */
  mdl->dat = NULL;

  /* free the logistic parameters and temporary coefficients. */
  vector_free(mdl->tmp);
  vector_free(mdl->xi);
  mdl->tmp = NULL;
  mdl->xi = NULL;

  /* reallocate the logistic parameters and temporary coefficients. */
  mdl->tmp = vector_alloc(model_tmp(mdl));
  mdl->xi = vector_alloc(dat->N);
  if (!mdl->tmp || !mdl->xi)
    return 0;

  /* initialize the logistic parameters. */
  vector_set_all(mdl->xi, 1.0);

  /* store the new dataset. */
  mdl->dat = dat;

  /* return succes. */
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
int model_add_factor (model_t *mdl, factor_t *f) {
  /* check the input pointers. */
  if (!mdl || !f)
    return 0;

  /* determine the new sizes of the model. */
  const unsigned int D = (mdl->D > f->D ? mdl->D : f->D);
  const unsigned int P = mdl->P + f->P;
  const unsigned int K = mdl->K + f->K;
  const unsigned int M = mdl->M + 1;

  /* reallocate the array of posterior factors. */
  factor_t **factors = (factor_t**)
    realloc(mdl->factors, M * sizeof(factor_t*));

  /* check that reallocation was successful. */
  if (!factors)
    return 0;

  /* reallocate the array of prior factors. */
  factor_t **priors = (factor_t**)
    realloc(mdl->priors, M * sizeof(factor_t*));

  /* check that reallocation was successful. */
  if (!priors) {
    mdl->factors = factors;
    return 0;
  }

  /* store the factor arrays into the model. */
  mdl->factors = factors;
  mdl->priors = priors;

  /* store the new sizes into the model. */
  mdl->D = D;
  mdl->P = P;
  mdl->M = M;
  mdl->K = K;

  /* store the posterior factor and make a copy for the prior. */
  mdl->factors[M - 1] = f;
  mdl->priors[M - 1] = factor_copy(f);

  /* check that factor duplication was successful. */
  if (!mdl->priors[M - 1])
    return 0;

  /* reallocate the weight means. */
  vector_free(mdl->wbar);
  mdl->wbar = vector_alloc(K);
  if (!mdl->wbar)
    return 0;

  /* reallocate the weight covariances. */
  matrix_free(mdl->Sigma);
  mdl->Sigma = matrix_alloc(K, K);
  if (!mdl->Sigma)
    return 0;

  /* reallocate the weight precisions. */
  matrix_free(mdl->Sinv);
  mdl->Sinv = matrix_alloc(K, K);
  if (!mdl->Sinv)
    return 0;

  /* reallocate the weight cholesky factors. */
  matrix_free(mdl->L);
  mdl->L = matrix_alloc(K, K);
  if (!mdl->L)
    return 0;

  /* reallocate the projections. */
  vector_free(mdl->h);
  mdl->h = vector_alloc(K);
  if (!mdl->h)
    return 0;

  /* reallocate the temporary vector. */
  vector_free(mdl->tmp);
  mdl->tmp = vector_alloc(model_tmp(mdl));
  if (!mdl->tmp)
    return 0;

  /* return success. */
  return 1;
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
double model_eval (const model_t *mdl, const vector_t *x,
                   const unsigned int p) {
  /* check the input pointers. */
  if (!mdl || !x)
    return 0.0;

  /* compute the model estimate. */
  double mode = 0.0;
  for (unsigned int j = 0, i = 0; j < mdl->M; j++) {
    /* get the current factor. */
    const factor_t *fj = mdl->factors[j];

    /* loop over the weights assigned to the factor. */
    for (unsigned int k = 0; k < fj->K; k++, i++)
      mode += vector_get(mdl->wbar, i) * factor_eval(fj, x, p, k);
  }

  /* return computed estimate. */
  return mode;
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
double model_mean (const model_t *mdl,
                   const vector_t *x, const unsigned int p,
                   const unsigned int j, const unsigned int k) {
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
double model_var (const model_t *mdl,
                  const vector_t *x, const unsigned int p,
                  const unsigned int j1, const unsigned int j2,
                  const unsigned int k1, const unsigned int k2) {
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
double model_cov (const model_t *mdl,
                  const vector_t *x1,
                  const vector_t *x2,
                  const unsigned int p1,
                  const unsigned int p2) {
  /* check the input pointers. */
  if (!mdl || !x1 || !x2)
    return 0.0;

  /* initialize the computation. */
  double cov = 0.0;

  /* sum together the contributions from each factor. */
  for (unsigned int j = 0; j < mdl->M; j++)
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
char *model_kernel (const model_t *mdl) {
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
  for (unsigned int j = 0, pj = 2; j < mdl->M; j++) {
    /* get the current factor string. */
    const factor_t *fj = mdl->factors[j];
    fstr[j] = factor_kernel(fj, pj);

    /* check for failure. */
    if (!fstr[j])
      return NULL;

    /* advance the factor parameter offset. */
    pj += fj->P;
  }

  /* determine the length of the kernel code string. */
  unsigned int len = 8;
  for (unsigned int j = 0; j < mdl->M; j++)
    len += strlen(fmt) + strlen(fstr[j]);

  /* allocate the kernel code string. */
  char *kstr = malloc(len);
  if (!kstr)
    return NULL;

  /* write each factor string. */
  char *pos = kstr;
  for (unsigned int j = 0; j < mdl->M; j++) {
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
double model_bound (const model_t *mdl) {
  /* check the input pointer. */
  if (!mdl)
    return 0.0;

  /* check the function pointer. */
  model_bound_fn bound_fn = MODEL_TYPE(mdl)->bound;
  if (!bound_fn)
    return 0.0;

  /* initialize a variable to hold the divergence penalty. */
  double div = 0.0;

  /* include the divergences of each factor into the penalty. */
  for (unsigned int j = 0; j < mdl->M; j++)
    div += factor_div(mdl->factors[j], mdl->priors[j]);

  /* execute the assigned bound function and
   * include the divergence penalty.
   */
  return bound_fn(mdl) - div;
}

/* model_predict(): return the model posterior prediction.
 *  - see model_predict_fn() for more information.
 */
int model_predict (const model_t *mdl, const vector_t *x,
                   const unsigned int p,
                   double *mean,
                   double *var) {
  /* check the input pointers. */
  if (!mdl || !x || !mean || !var)
    return 0;

  /* check the function pointer. */
  model_predict_fn predict_fn = MODEL_TYPE(mdl)->predict;
  if (!predict_fn)
    return 0;

  /* check the vector size. */
  if (x->len < mdl->D)
    return 0;

  /* execute the assigned prediction function. */
  return predict_fn(mdl, x, p, mean, var);
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
int model_predict_all (const model_t *mdl,
                       data_t *mean,
                       data_t *var) {
  /* declare required variables:
   *  @mu: individual means.
   *  @eta: individual variances.
   */
  double mu, eta;

  /* check the input pointers. */
  if (!mdl || !mean || !var)
    return 0;

  /* check that the structures have matching dimensionality. */
  if (mean->D != mdl->D || var->D != mdl->D || mean->N != var->N)
    return 0;

  /* loop over each observation. */
  for (unsigned int i = 0; i < mean->N; i++) {
    /* compute the posterior mean and variance. */
    model_predict(mdl, mean->data[i].x, mean->data[i].p, &mu, &eta);

    /* store the predictions. */
    mean->data[i].y = mu;
    var->data[i].y = eta;
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
int model_reset (model_t *mdl) {
  /* check the input pointer. */
  if (!mdl)
    return 0;

  /* loop over the factors, copying parameters from the priors. */
  for (unsigned int j = 0; j < mdl->M; j++) {
    /* set the factor parameters from the prior. */
    const factor_t *fp = mdl->priors[j];
    if (!model_set_parms(mdl, j, fp->par))
      return 0;
  }

  /* return success. */
  return model_infer(mdl);
}

/* model_infer(): fully update the nuisance parameters of a model.
 *  - see model_infer_fn() for more information.
 */
int model_infer (model_t *mdl) {
  /* check the input pointer. */
  if (!mdl)
    return 0;

  /* check the function pointer. */
  model_infer_fn infer_fn = MODEL_TYPE(mdl)->infer;
  if (!infer_fn)
    return 0;

  /* execute the assigned inference function. */
  return infer_fn(mdl);
}

/* model_update(): efficiently update the nuisance parameters of a model.
 *  - see model_update_fn() for more information.
 */
int model_update (model_t *mdl, const unsigned int j) {
  /* check the input pointer and factor index. */
  if (!mdl || j >= mdl->M)
    return 0;

  /* if an update function is assigned, execute it. */
  model_update_fn update_fn = MODEL_TYPE(mdl)->update;
  if (update_fn && update_fn(mdl, j))
    return 1;

  /* fall back to the infer function, if assigned. */
  model_infer_fn infer_fn = MODEL_TYPE(mdl)->infer;
  if (infer_fn)
    return infer_fn(mdl);

  /* neither function is assigned. fail. */
  return 0;
}

/* model_gradient(): return the gradient of the lower bound.
 *  - see model_gradient_fn() for more information.
 */
int model_gradient (const model_t *mdl, const unsigned int i,
                    const unsigned int j, vector_t *grad) {
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
  model_gradient_fn gradient_fn = MODEL_TYPE(mdl)->gradient;
  if (!gradient_fn)
    return 0;

  /* execute the assigned gradient function. */
  return gradient_fn(mdl, i, j, grad);
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
int model_meanfield (const model_t *mdl, const unsigned int j) {
  /* check the input pointers and factor index. */
  if (!mdl || !mdl->dat || j >= mdl->M)
    return 0;

  /* gain access to the factor and its number of weights and parameters. */
  factor_t *f = mdl->factors[j];
  const unsigned int K = f->K;
  const unsigned int P = f->P;

  /* check if the factor has no parameters. */
  if (P == 0)
    return 1;

  /* check the model and factor function pointers. */
  model_meanfield_fn mdl_fn = MODEL_TYPE(mdl)->meanfield;
  factor_meanfield_fn fac_fn = FACTOR_TYPE(f)->meanfield;
  if (!mdl_fn || !fac_fn)
    return 0;

  /* gain access to the associated prior. */
  const factor_t *fp = mdl->priors[j];

  /* create sets of coefficients for mean-field updates. */
  vector_view_t b = vector_view_array(mdl->tmp->data, K);
  matrix_view_t B = matrix_view_array(mdl->tmp->data + K, K, K);

  /* initialize the factor update. */
  if (!fac_fn(f, NULL, NULL, NULL, NULL))
    return 0;

  /* loop over each data point. */
  datum_t *di = mdl->dat->data;
  const unsigned int N = mdl->dat->N;
  for (unsigned int i = 0; i < N; i++, di++) {
    /* 1. compute the coefficients of the data point.
     * 2. stream the data point and coefficients to the factor.
     */
    mdl_fn(mdl, i, j, &b, &B);
    fac_fn(f, fp, di, &b, &B);
  }

  /* finalize the factor update. */
  return fac_fn(f, fp, NULL, NULL, NULL);
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
unsigned int model_weight_idx (const model_t *mdl,
                               const unsigned int j,
                               const unsigned int k) {
  /* check the input pointers and indices. */
  if (!mdl || j >= mdl->M || k >= mdl->factors[j]->K)
    return 0;

  /* compute the weight offset for the specified factor. */
  unsigned int idx = 0;
  for (unsigned int j2 = 0; j2 < j; j2++)
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
void model_weight_adjust_init (const model_t *mdl, const unsigned int j) {
  /* get the weight offset and count of the current factor. */
  const unsigned int k0 = model_weight_idx(mdl, j, 0);
  const unsigned int K = mdl->factors[j]->K;

  /* declare vector views for tracking precision matrix modifications. */
  vector_view_t u;
  matrix_view_t U;

  /* create the matrix view for storing updates. */
  U = matrix_view_array(mdl->tmp->data + mdl->K + mdl->P, K, mdl->K);

  /* copy the initial rows of the precision matrix. */
  for (unsigned int k = 0; k < K; k++) {
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
int model_weight_adjust (model_t *mdl, const unsigned int j) {
  /* get the weight offset and count of the current factor. */
  const unsigned int k0 = model_weight_idx(mdl, j, 0);
  const unsigned int K = mdl->factors[j]->K;

  /* declare vector views for tracking precision matrix modifications. */
  vector_view_t u, v, z;
  matrix_view_t U, V;

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
  for (unsigned int k = 0; k < K; k++) {
    v = matrix_row(&V, k);
    matrix_copy_row(&v, mdl->Sinv, k0 + k);
  }

  /* compute the difference between the precision matrix rows. */
  matrix_sub(&V, &U);

  /* compute the magnitude of the differences. */
  double vss = 0.0;
  for (unsigned int k = 0; k < K; k++) {
    v = matrix_row(&V, k);
    vss += blas_ddot(&v, &v);
  }

  /* fail if the differences have zero effective magnitude. */
  if (vss == 0.0)
    return 0;

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

  /* return success. */
  return 1;
}

