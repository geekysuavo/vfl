
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

  /* in order to conserve memory, determine the largest number of
   * factor weights that will be updated at the same time.
   */
  kmax = 0;
  for (unsigned int j = 0; j < mdl->M; j++)
    kmax = (mdl->factors[j]->K > kmax ? mdl->factors[j]->K : kmax);

  /* the scalars are laid out as follows:
   *  z: (K, 1)
   *  U: (max(k), K)
   *  V: (max(k), K)
   */
  ntmp = mdl->K + mdl->P + 2 * kmax * mdl->K;

  /* return the computed scalar count. */
  return ntmp;
}

/* model_alloc(): allocate a new empty variational feature model.
 *
 * returns:
 *  newly allocated and initialized model structure pointer.
 */
model_t *model_alloc (void) {
  /* allocate the structure pointer. */
  model_t *mdl = malloc(sizeof(model_t));
  if (!mdl)
    return NULL;

  /* initialize the sizes of the model. */
  mdl->D = 0;
  mdl->P = 0;
  mdl->M = 0;
  mdl->K = 0;

  /* initialize the function pointers. */
  mdl->bound = NULL;
  mdl->predict = NULL;
  mdl->infer = NULL;
  mdl->update = NULL;
  mdl->grad = NULL;

  /* initialize the prior parameters. */
  mdl->alpha0 = 1.0;
  mdl->beta0 = 1.0;
  mdl->nu = 1.0;

  /* initialize the posterior noise parameters. */
  mdl->alpha = mdl->alpha0;
  mdl->beta = mdl->beta0;

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

  /* free the associated dataset. */
  data_free(mdl->dat);

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
  /* check the input pointers. */
  if (!mdl || !dat)
    return 0;

  /* if the dataset and model are non-empty, make
   * sure they are compatible with each other.
   */
  if (mdl->D && dat->N && dat->D < mdl->D)
    return 0;

  /* free the existing dataset and store the new dataset. */
  data_free(mdl->dat);
  mdl->dat = dat;

  /* reallocate the logistic parameters. */
  vector_free(mdl->xi);
  mdl->xi = vector_alloc(dat->N);
  if (!mdl->xi)
    return 0;

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

/* model_mean(): return the first moment of a model basis element.
 *
 * arguments:
 *  @mdl: model structure pointer.
 *  @x: observation input vector.
 *  @j: factor index.
 *  @k: basis index.
 *
 * returns:
 *  expectation of the requested basis element.
 */
double model_mean (const model_t *mdl, const vector_t *x,
                   const unsigned int j, const unsigned int k) {
  /* check the input pointers and indices. */
  if (!mdl || !x || j >= mdl->M || k >= mdl->factors[j]->K)
    return 0.0;

  /* return the requested expectation. */
  return factor_mean(mdl->factors[j], x, k);
}

/* model_var(): return the second moment of a pair of
 * model basis elements.
 *
 * arguments:
 *  @mdl: model structure pointer.
 *  @x: observation input vector.
 *  @j1: first factor index.
 *  @j2: second factor index.
 *  @k1: first basis index.
 *  @k2: second basis index.
 *
 * returns:
 *  expectation of the requested product of basis elements.
 */
double model_var (const model_t *mdl, const vector_t *x,
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
    return factor_mean(mdl->factors[j1], x, k1) *
           factor_mean(mdl->factors[j2], x, k2);

  /* return the requested expectation. */
  return factor_var(mdl->factors[j1], x, k1, k2);
}

/* model_bound(): return the model variational lower bound.
 *  - see model_bound_fn() for more information.
 */
double model_bound (const model_t *mdl) {
  /* check the input pointers. */
  if (!mdl || !mdl->bound)
    return 0.0;

  /* initialize a variable to hold the divergence penalty. */
  double div = 0.0;

  /* include the divergences of each factor into the penalty. */
  for (unsigned int j = 0; j < mdl->M; j++)
    div += factor_div(mdl->factors[j], mdl->priors[j]);

  /* execute the assigned bound function and
   * include the divergence penalty.
   */
  return mdl->bound(mdl) - div;
}

/* model_predict(): return the model posterior prediction.
 *  - see model_predict_fn() for more information.
 */
int model_predict (const model_t *mdl, const vector_t *x,
                   double *mean, double *var) {
  /* check the input pointers and sizes. */
  if (!mdl || !mdl->predict || !x || !mean || !var || x->len < mdl->D)
    return 0;

  /* execute the assigned prediction function. */
  return mdl->predict(mdl, x, mean, var);
}

/* model_infer(): fully update the nuisance parameters of a model.
 *  - see model_infer_fn() for more information.
 */
int model_infer (model_t *mdl) {
  /* check the input pointers. */
  if (!mdl || !mdl->infer)
    return 0;

  /* execute the assigned inference function. */
  return mdl->infer(mdl);
}

/* model_update(): efficiently update the nuisance parameters of a model.
 *  - see model_update_fn() for more information.
 */
int model_update (model_t *mdl, const unsigned int j) {
  /* check the input pointer and factor index. */
  if (!mdl || j >= mdl->M)
    return 0;

  /* if an update function is assigned, execute it. */
  if (mdl->update)
    return mdl->update(mdl, j);

  /* fall back to the infer function, if assigned. */
  if (mdl->infer)
    return mdl->infer(mdl);

  /* neither function is assigned. fail. */
  return 0;
}

/* model_gradient(): return the gradient of the lower bound.
 *  - see model_gradient_fn() for more information.
 */
int model_gradient (const model_t *mdl, const unsigned int i,
                    const unsigned int j, vector_t *grad) {
  /* check the input pointers and factor index. */
  if (!mdl || !mdl->dat || !mdl->grad || !grad ||
      i >= mdl->dat->N || j >= mdl->M ||
      grad->len != mdl->factors[j]->P)
    return 0;

  /* execute the assigned gradient function. */
  return mdl->grad(mdl, i, j, grad);
}

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

