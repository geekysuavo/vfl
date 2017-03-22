
/* ensure once-only inclusion. */
#ifndef __VFL_MODEL_H__
#define __VFL_MODEL_H__

/* include c headers. */
#include <stdlib.h>
#include <math.h>

/* include application headers. */
#include <vfl/data.h>
#include <vfl/factor.h>
#include <vfl/util/chol.h>

/* model_t: defined type for the model structure. */
typedef struct model model_t;

/* model_bound_fn(): return the lower bound on the log-evidence given
 * the currently inferred nuisance parameters.
 *
 * arguments:
 *  @mdl: model structure pointer.
 *
 * returns:
 *  current value of the variational lower bound.
 */
typedef double (*model_bound_fn) (const model_t *mdl);

/* MODEL_BOUND(): macro function for declaring and defining
 * functions conforming to model_bound_fn().
 */
#define MODEL_BOUND(name) \
double name ## _bound (const model_t *mdl)

/* model_predict_fn(): return the posterior predicted mean and variance
 * of the model at a given observation input vector.
 *
 * arguments:
 *  @mdl: model structure pointer.
 *  @x: observation input vector.
 *  @p: function output index.
 *  @mean: pointer to the predicted mean.
 *  @var: pointer to the predicted variance.
 *
 * returns:
 *  integer indicating prediction success (1) or failure (0).
 */
typedef int (*model_predict_fn) (const model_t *mdl, const vector_t *x,
                                 const unsigned int p,
                                 double *mean,
                                 double *var);

/* MODEL_PREDICT(): macro function for declaring and defining
 * functions conforming to model_predict_fn().
 */
#define MODEL_PREDICT(name) \
int name ## _predict (const model_t *mdl, const vector_t *x, \
                      const unsigned int p, \
                      double *mean, \
                      double *var)

/* model_infer_fn(): update the posterior nuisance parameters of a model.
 *
 * arguments:
 *  @mdl: model structure pointer.
 *
 * returns:
 *  integer indicating inference success (1) or failure (0).
 */
typedef int (*model_infer_fn) (model_t *mdl);

/* MODEL_INFER(): macro function for declaring and defining
 * functions conforming to model_infer_fn().
 */
#define MODEL_INFER(name) \
int name ## _infer (model_t *mdl)

/* model_update_fn(): update the posterior nuisance parameters of a model
 * using low-rank updates to the inverse covariance matrix and its cholesky
 * factors.
 *
 * arguments:
 *  @mdl: model structure pointer.
 *  @j: updated factor index.
 *
 * returns:
 *  integer indicating inference success (1) or failure (0).
 */
typedef int (*model_update_fn) (model_t *mdl, const unsigned int j);

/* MODEL_UPDATE(): macro function for declaring and defining
 * functions conforming to model_update_fn().
 */
#define MODEL_UPDATE(name) \
int name ## _update (model_t *mdl, const unsigned int j)

/* model_gradient_fn(): return the gradient of the variational lower bound
 * with respect to the parameters of a single factor, taken against a
 * single observation in the model-associated dataset.
 *
 * arguments:
 *  @mdl: model structure pointer.
 *  @i: dataset observation index.
 *  @j: variational factor index.
 *  @grad: gradient vector.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
typedef int (*model_gradient_fn) (const model_t *mdl, const unsigned int i,
                                  const unsigned int j, vector_t *grad);

/* MODEL_GRADIENT(): macro function for declaring and defining
 * functions conforming to model_gradient_fn().
 */
#define MODEL_GRADIENT(name) \
int name ## _gradient (const model_t *mdl, const unsigned int i, \
                       const unsigned int j, vector_t *grad)

/* model_meanfield_fn(): perform an assumed-density mean-field update
 * of a single factor in a variational feature learning model.
 *
 * arguments:
 *  @mdl: model structure pointer.
 *  @j: variational factor index.
 *  @c: vector for storing first-order coefficients.
 *  @C: matrix for storing second-order coefficients.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
typedef int (*model_meanfield_fn) (const model_t *mdl, const unsigned int j,
                                   const vector_t *c, const matrix_t *C);

/* MODEL_MEANFIELD(): macro function for declaring and defining
 * functions conforming to model_meanfield_fn().
 */
#define MODEL_MEANFIELD(name) \
int name ## _meanfield (const model_t *mdl, const unsigned int j, \
                        const vector_t *c, const matrix_t *C)

/* struct model: structure for holding a variational feature model.
 */
struct model {
  /* model sizes:
   *  @D: number of dimensions.
   *  @P: number of parameters.
   *  @M: number of factors.
   *  @K: number of weights.
   */
  unsigned int D, P, M, K;

  /* model-specific functions:
   *  @bound: variational lower bound.
   *  @predict: predictive mean and variance.
   *  @infer: complete posterior nuisance inference.
   *  @update: partial posterior nuisance inference.
   *  @gradient: lower bound gradient computation.
   *  @meanfield: assumed-density mean-field computation.
   */
  model_bound_fn bound;
  model_predict_fn predict;
  model_infer_fn infer;
  model_update_fn update;
  model_gradient_fn gradient;
  model_meanfield_fn meanfield;

  /* prior parameters:
   *
   *  noise:
   *   @alpha0: noise precision, prior shape.
   *   @beta0: noise precision, prior rate.
   *
   *  weights:
   *   @nu: prior amplitude precision, relative to noise.
   */
  double alpha0, beta0, nu;

  /* posterior parameters:
   *
   *  noise:
   *   @alpha: noise precision, posterior shape.
   *   @beta: noise precision, posterior rate.
   *
   *  weights:
   *   @wbar: weight means.
   *   @Sigma: weight covariances.
   *
   *  logistic coefficients:
   *   @xi: variational parameters of the logistic functions.
   */
  double alpha, beta;
  vector_t *wbar;
  matrix_t *Sigma;
  vector_t *xi;

  /* intermediate variables:
   *  @Sinv: weight precisions.
   *  @L: weight precision cholesky factors.
   *  @h: projection vector.
   */
  matrix_t *Sinv;
  matrix_t *L;
  vector_t *h;

  /* variational heart of the model:
   *  @factors: array of variational features/factors to be inferred.
   *  @priors: array of feature priors to use during inference.
   */
  factor_t **factors;
  factor_t **priors;

  /* @dat: associated dataset for inference.
   */
  data_t *dat;

  /* @tmp: temporary vector used to store transient intermediates
   * during bound, inference and gradient calculations.
   */
  vector_t *tmp;
};

/* function declarations (model.c): */

model_t *model_alloc (void);

void model_free (model_t *mdl);

int model_set_alpha0 (model_t *mdl, const double alpha0);

int model_set_beta0 (model_t *mdl, const double beta0);

int model_set_nu (model_t *mdl, const double nu);

int model_set_parms (model_t *mdl, const unsigned int j,
                     const vector_t *par);

int model_set_data (model_t *mdl, data_t *dat);

int model_add_factor (model_t *mdl, factor_t *f);

double model_mean (const model_t *mdl,
                   const vector_t *x, const unsigned int p,
                   const unsigned int j, const unsigned int k);

double model_var (const model_t *mdl,
                  const vector_t *x, const unsigned int p,
                  const unsigned int j1, const unsigned int j2,
                  const unsigned int k1, const unsigned int k2);

double model_bound (const model_t *mdl);

int model_predict (const model_t *mdl, const vector_t *x,
                   const unsigned int p,
                   double *mean,
                   double *var);

int model_predict_all (const model_t *mdl,
                       data_t *mean,
                       data_t *var);

int model_infer (model_t *mdl);

int model_update (model_t *mdl, const unsigned int j);

int model_gradient (const model_t *mdl, const unsigned int i,
                    const unsigned int j, vector_t *grad);

int model_meanfield (const model_t *mdl, const unsigned int j);

/* global, yet internally used function declarations (model.c): */

unsigned int model_weight_idx (const model_t *mdl,
                               const unsigned int j,
                               const unsigned int k);

void model_weight_adjust_init (const model_t *mdl, const unsigned int j);

int model_weight_adjust (model_t *mdl, const unsigned int j);

/* derived model headers: */
#include <vfl/model/vfc.h>
#include <vfl/model/vfr.h>
#include <vfl/model/tauvfr.h>

#endif /* !__VFL_MODEL_H__ */

