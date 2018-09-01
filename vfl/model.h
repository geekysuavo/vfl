
/* ensure once-only inclusion. */
#ifndef __VFL_MODEL_H__
#define __VFL_MODEL_H__

/* include vfl headers. */
#include <vfl/util/chol.h>
#include <vfl/factor.h>

/* Model_Check(): macro to check if a PyObject is a Model.
 */
#define Model_Check(v) (Py_TYPE(v) == &Model_Type)

/* Model_Type: globally available model type structure.
 */
PyAPI_DATA(PyTypeObject) Model_Type;

/* Model: defined type for the model structure. */
typedef struct model Model;

/* model_init_fn(): initialize a model structure
 * in a type-specific manner.
 *
 * arguments:
 *  @mdl: model structure pointer to initialize.
 *
 * returns:
 *  integer indicating initialization success (1) or failure (0).
 */
typedef int (*model_init_fn) (Model *mdl);

/* model_bound_fn(): return the lower bound on the log-evidence given
 * the currently inferred nuisance parameters.
 *
 * arguments:
 *  @mdl: model structure pointer.
 *
 * returns:
 *  current value of the variational lower bound.
 */
typedef double (*model_bound_fn) (const Model *mdl);

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
typedef int (*model_predict_fn) (const Model *mdl, const Vector *x,
                                 size_t p, double *mean, double *var);

/* model_infer_fn(): update the posterior nuisance parameters of a model.
 *
 * arguments:
 *  @mdl: model structure pointer.
 *
 * returns:
 *  integer indicating inference success (1) or failure (0).
 */
typedef int (*model_infer_fn) (Model *mdl);

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
typedef int (*model_update_fn) (Model *mdl, size_t j);

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
typedef int (*model_gradient_fn) (const Model *mdl, size_t i, size_t j,
                                  Vector *grad);

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
typedef int (*model_meanfield_fn) (const Model *mdl, size_t i, size_t j,
                                   Vector *b, Matrix *B);

/* MODEL_INIT(): macro function for declaring and defining
 * functions conforming to model_init_fn().
 */
#define MODEL_INIT(name) \
int name ## _init (Model *mdl)

/* MODEL_BOUND(): macro function for declaring and defining
 * functions conforming to model_bound_fn().
 */
#define MODEL_BOUND(name) \
double name ## _bound (const Model *mdl)

/* MODEL_PREDICT(): macro function for declaring and defining
 * functions conforming to model_predict_fn().
 */
#define MODEL_PREDICT(name) \
int name ## _predict (const Model *mdl, const Vector *x, \
                      size_t p, double *mean, double *var)

/* MODEL_INFER(): macro function for declaring and defining
 * functions conforming to model_infer_fn().
 */
#define MODEL_INFER(name) \
int name ## _infer (Model *mdl)

/* MODEL_UPDATE(): macro function for declaring and defining
 * functions conforming to model_update_fn().
 */
#define MODEL_UPDATE(name) \
int name ## _update (Model *mdl, size_t j)

/* MODEL_GRADIENT(): macro function for declaring and defining
 * functions conforming to model_gradient_fn().
 */
#define MODEL_GRADIENT(name) \
int name ## _gradient (const Model *mdl, size_t i, size_t j, \
                       Vector *grad)

/* MODEL_MEANFIELD(): macro function for declaring and defining
 * functions conforming to model_meanfield_fn().
 */
#define MODEL_MEANFIELD(name) \
int name ## _meanfield (const Model *mdl, size_t i, size_t j, \
                        Vector *b, Matrix *B)

/* struct model: structure for holding a variational feature model.
 */
struct model {
  /* object base. */
  PyObject_HEAD

  /* model specialization functions:
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

  /* model sizes:
   *  @D: number of dimensions.
   *  @P: number of parameters.
   *  @M: number of factors.
   *  @K: number of weights.
   */
  size_t D, P, M, K;

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
  Vector *wbar;
  Matrix *Sigma;
  Vector *xi;

  /* intermediate variables:
   *  @Sinv: weight precisions.
   *  @L: weight precision cholesky factors.
   *  @h: projection vector.
   */
  Matrix *Sinv;
  Matrix *L;
  Vector *h;

  /* variational heart of the model:
   *  @factors: array of variational features/factors to be inferred.
   *  @priors: array of feature priors to use during inference.
   */
  Factor **factors;
  Factor **priors;

  /* @dat: associated dataset for inference.
   */
  Data *dat;

  /* @tmp: temporary vector used to store transient intermediates
   *       during bound, inference and gradient calculations.
   */
  Vector *tmp;
};

/* function declarations (model-obj.c): */
/*

#define model_alloc(T) \
  (model_t*) obj_alloc((object_type_t*) T)

int model_init (model_t *mdl);

void model_free (model_t *mdl);

object_t *model_getprop_dims (const model_t *mdl);

object_t *model_getprop_pars (const model_t *mdl);

object_t *model_getprop_cmps (const model_t *mdl);

object_t *model_getprop_wgts (const model_t *mdl);

object_t *model_getprop_bound (const model_t *mdl);

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

*/
/* function declarations (model.c): */
/*

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

*/
/* global, yet internally used function declarations (model.c): */
/*

unsigned int model_weight_idx (const model_t *mdl,
                               const unsigned int j,
                               const unsigned int k);

void model_weight_adjust_init (const model_t *mdl, const unsigned int j);

int model_weight_adjust (model_t *mdl, const unsigned int j);

*/
/* function declarations (model/tauvfr.c): */
/*

int tauvfr_set_tau (model_t *mdl, const double tau);
*/

#endif /* !__VFL_MODEL_H__ */

