
/* ensure once-only inclusion. */
#ifndef __VFL_MODEL_H__
#define __VFL_MODEL_H__

/* include c headers. */
#include <stdlib.h>
#include <math.h>

/* include vfl headers. */
#include <vfl/util/chol.h>
#include <vfl/factor.h>

/* OBJECT_IS_MODEL(): check if an object is a model.
 */
#define OBJECT_IS_MODEL(obj) \
  (OBJECT_TYPE(obj)->init == (object_init_fn) model_init)

/* MODEL_TYPE(): macro function for casting model structure pointers
 * to their associated type structures.
 */
#define MODEL_TYPE(s) ((model_type_t*) (s)->type)

/* MODEL_PROP_BASE: base set of object properties available
 * to all models.
 */
#define MODEL_PROP_BASE \
  { "D", (object_getprop_fn) model_getprop_dims, NULL }, \
  { "P", (object_getprop_fn) model_getprop_pars, NULL }, \
  { "M", (object_getprop_fn) model_getprop_cmps, NULL }, \
  { "K", (object_getprop_fn) model_getprop_wgts, NULL }, \
  { "wbar", \
    (object_getprop_fn) model_getprop_wmean, \
    (object_setprop_fn) model_setprop_wmean }, \
  { "Sigma", \
    (object_getprop_fn) model_getprop_wcov, \
    (object_setprop_fn) model_setprop_wcov }, \
  { "data", \
    (object_getprop_fn) model_getprop_data, \
    (object_setprop_fn) model_setprop_data }, \
  { "factors", \
    (object_getprop_fn) model_getprop_factors, \
    (object_setprop_fn) model_setprop_factors }

/* MODEL_METHOD_BASE: base set of object methods available
 * to all models.
 */
#define MODEL_METHOD_BASE \
  { "add", (object_method_fn) model_method_add }, \
  { "reset", (object_method_fn) model_method_reset }, \
  { "infer", (object_method_fn) model_method_infer }, \
  { "eval", (object_method_fn) model_method_eval }, \
  { "predict", (object_method_fn) model_method_predict }

/* MODEL_PROP_ALPHA0: property array entry for prior noise shape.
 */
#define MODEL_PROP_ALPHA0 \
  { "alpha0", \
    (object_getprop_fn) model_getprop_alpha0, \
    (object_setprop_fn) model_setprop_alpha0 }

/* MODEL_PROP_BETA0: property array entry for prior noise rate.
 */
#define MODEL_PROP_BETA0 \
  { "beta0", \
    (object_getprop_fn) model_getprop_beta0, \
    (object_setprop_fn) model_setprop_beta0 }

/* MODEL_PROP_ALPHA: property array entry for posterior noise shape.
 */
#define MODEL_PROP_ALPHA \
  { "alpha", (object_getprop_fn) model_getprop_alpha, NULL }

/* MODEL_PROP_BETA: property array entry for posterior noise rate.
 */
#define MODEL_PROP_BETA \
  { "beta", (object_getprop_fn) model_getprop_beta, NULL }

/* MODEL_PROP_TAU: property array entry for read/write noise precision.
 */
#define MODEL_PROP_TAU \
  { "tau", \
    (object_getprop_fn) model_getprop_tau, \
    (object_setprop_fn) model_setprop_tau }

/* MODEL_PROP_TAU_READONLY: property array entry for
 * read-only noise precision.
 */
#define MODEL_PROP_TAU_READONLY \
  { "tau", (object_getprop_fn) model_getprop_tau, NULL }

/* MODEL_PROP_NU: property entry for relative noise/weight precision.
 */
#define MODEL_PROP_NU \
  { "nu", \
    (object_getprop_fn) model_getprop_nu, \
    (object_setprop_fn) model_setprop_nu }

/* model_t: defined type for the model structure. */
typedef struct model model_t;

/* model_init_fn(): initialize a model structure
 * in a type-specific manner.
 *
 * arguments:
 *  @mdl: model structure pointer to initialize.
 *
 * returns:
 *  integer indicating initialization success (1) or failure (0).
 */
typedef int (*model_init_fn) (model_t *mdl);

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

/* model_infer_fn(): update the posterior nuisance parameters of a model.
 *
 * arguments:
 *  @mdl: model structure pointer.
 *
 * returns:
 *  integer indicating inference success (1) or failure (0).
 */
typedef int (*model_infer_fn) (model_t *mdl);

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

/* model_meanfield_fn(): perform an assumed-density mean-field update
 * of a single factor in a variational feature learning model.
 *
 * arguments:
 *  @mdl: model structure pointer.
 *  @i: dataset observation index.
 *  @j: variational factor index.
 *  @b: vector for storing first-order coefficients.
 *  @B: matrix for storing second-order coefficients.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
typedef int (*model_meanfield_fn) (const model_t *mdl,
                                   const unsigned int i,
                                   const unsigned int j,
                                   vector_t *b, matrix_t *B);

/* MODEL_INIT(): macro function for declaring and defining
 * functions conforming to model_init_fn().
 */
#define MODEL_INIT(name) \
int name ## _init (model_t *mdl)

/* MODEL_BOUND(): macro function for declaring and defining
 * functions conforming to model_bound_fn().
 */
#define MODEL_BOUND(name) \
double name ## _bound (const model_t *mdl)

/* MODEL_PREDICT(): macro function for declaring and defining
 * functions conforming to model_predict_fn().
 */
#define MODEL_PREDICT(name) \
int name ## _predict (const model_t *mdl, const vector_t *x, \
                      const unsigned int p, \
                      double *mean, \
                      double *var)

/* MODEL_INFER(): macro function for declaring and defining
 * functions conforming to model_infer_fn().
 */
#define MODEL_INFER(name) \
int name ## _infer (model_t *mdl)

/* MODEL_UPDATE(): macro function for declaring and defining
 * functions conforming to model_update_fn().
 */
#define MODEL_UPDATE(name) \
int name ## _update (model_t *mdl, const unsigned int j)

/* MODEL_GRADIENT(): macro function for declaring and defining
 * functions conforming to model_gradient_fn().
 */
#define MODEL_GRADIENT(name) \
int name ## _gradient (const model_t *mdl, const unsigned int i, \
                       const unsigned int j, vector_t *grad)

/* MODEL_MEANFIELD(): macro function for declaring and defining
 * functions conforming to model_meanfield_fn().
 */
#define MODEL_MEANFIELD(name) \
int name ## _meanfield (const model_t *mdl, \
                        const unsigned int i, \
                        const unsigned int j, \
                        vector_t *b, matrix_t *B)

/* model_type_t: structure for holding type-specific model information.
 */
typedef struct {
  /* @base: basic object type information. */
  object_type_t base;

  /* model type-specific functions:
   *  @init: initialization.
   *  @bound: variational lower bound.
   *  @predict: predictive mean and variance.
   *  @infer: complete posterior nuisance inference.
   *  @update: partial posterior nuisance inference.
   *  @gradient: lower bound gradient computation.
   *  @meanfield: assumed-density mean-field computation.
   */
  model_init_fn init;
  model_bound_fn bound;
  model_predict_fn predict;
  model_infer_fn infer;
  model_update_fn update;
  model_gradient_fn gradient;
  model_meanfield_fn meanfield;
}
model_type_t;

/* struct model: structure for holding a variational feature model.
 */
struct model {
  /* base structure members. */
  OBJECT_BASE;

  /* model sizes:
   *  @D: number of dimensions.
   *  @P: number of parameters.
   *  @M: number of factors.
   *  @K: number of weights.
   */
  unsigned int D, P, M, K;

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
   *   @tau: noise precision.
   *
   *  weights:
   *   @wbar: weight means.
   *   @Sigma: weight covariances.
   *
   *  logistic coefficients:
   *   @xi: variational parameters of the logistic functions.
   */
  double alpha, beta, tau;
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

/* function declarations (model-obj.c): */

#define model_alloc(T) \
  (model_t*) obj_alloc((object_type_t*) T)

int model_init (model_t *mdl);

void model_free (model_t *mdl);

object_t *model_getprop_dims (const model_t *mdl);

object_t *model_getprop_pars (const model_t *mdl);

object_t *model_getprop_cmps (const model_t *mdl);

object_t *model_getprop_wgts (const model_t *mdl);

object_t *model_getprop_wmean (const model_t *mdl);

object_t *model_getprop_wcov (const model_t *mdl);

data_t *model_getprop_data (const model_t *mdl);

object_t *model_getprop_factors (const model_t *mdl);

object_t *model_getprop_alpha0 (const model_t *mdl);

object_t *model_getprop_beta0 (const model_t *mdl);

object_t *model_getprop_alpha (const model_t *mdl);

object_t *model_getprop_beta (const model_t *mdl);

object_t *model_getprop_tau (const model_t *mdl);

object_t *model_getprop_nu (const model_t *mdl);

int model_setprop_wmean (model_t *mdl, object_t *val);

int model_setprop_wcov (model_t *mdl, object_t *val);

int model_setprop_data (model_t *mdl, object_t *val);

int model_setprop_factors (model_t *mdl, object_t *val);

int model_setprop_alpha0 (model_t *mdl, object_t *val);

int model_setprop_beta0 (model_t *mdl, object_t *val);

int model_setprop_tau (model_t *mdl, object_t *val);

int model_setprop_nu (model_t *mdl, object_t *val);

object_t *model_method_add (model_t *mdl, object_t *args);

object_t *model_method_infer (model_t *mdl, object_t *args);

object_t *model_method_reset (model_t *mdl, object_t *args);

object_t *model_method_eval (model_t *mdl, object_t *args);

object_t *model_method_predict (model_t *mdl, object_t *args);

/* function declarations (model.c): */

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

double model_cov (const model_t *mdl,
                  const vector_t *x1,
                  const vector_t *x2,
                  const unsigned int p1,
                  const unsigned int p2);

char *model_kernel (const model_t *mdl);

double model_bound (const model_t *mdl);

double model_eval (const model_t *mdl, const vector_t *x,
                   const unsigned int p);

int model_predict (const model_t *mdl, const vector_t *x,
                   const unsigned int p,
                   double *mean,
                   double *var);

int model_eval_all (const model_t *mdl, data_t *dat);

int model_predict_all (const model_t *mdl, data_t *mean, data_t *var);

int model_reset (model_t *mdl);

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

/* function declarations (model/tauvfr.c): */

int tauvfr_set_tau (model_t *mdl, const double tau);

/* available model types: */

extern const model_type_t *vfl_model_vfc;
extern const model_type_t *vfl_model_vfr;
extern const model_type_t *vfl_model_tauvfr;

#endif /* !__VFL_MODEL_H__ */

