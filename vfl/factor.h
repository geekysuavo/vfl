
/* ensure once-only inclusion. */
#ifndef __VFL_FACTOR_H__
#define __VFL_FACTOR_H__

/* include c headers. */
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

/* include vfl headers. */
#include <vfl/util/specfun.h>
#include <vfl/util/blas.h>
#include <vfl/data.h>

/* OBJECT_IS_FACTOR(): check if an object is a factor.
 */
#define OBJECT_IS_FACTOR(obj) \
  (OBJECT_TYPE(obj)->init == (object_init_fn) factor_init)

/* FACTOR_TYPE(): macro function for casting factor structure pointers
 * to their associated type structures.
 */
#define FACTOR_TYPE(s) ((factor_type_t*) (s)->type)

/* FACTOR_PROP_BASE: base set of object properties available
 * to all factors.
 */
#define FACTOR_PROP_BASE \
  { "dim", \
    (object_getprop_fn) factor_getprop_dim, \
    (object_setprop_fn) factor_setprop_dim }, \
  { "fixed", \
    (object_getprop_fn) factor_getprop_fixed, \
    (object_setprop_fn) factor_setprop_fixed }

/* FACTOR_METHOD_BASE: base set of object methods available
 * to all factors.
 */
#define FACTOR_METHOD_BASE \
  { "set", (object_method_fn) factor_setprop }

/* FACTOR_PROP(): macro function for inserting a static factor
 * property into an object properties array.
 */
#define FACTOR_PROP(typ,name) \
  { #name, \
    (object_getprop_fn) typ ## obj_getparm_ ## name, \
    (object_setprop_fn) typ ## obj_setparm_ ## name }

/* FACTOR_PROP_GETSET(): macro function for defining property
 * getter and setter functions for a static factor property.
 */
#define FACTOR_PROP_GETSET(typ,name,idx) \
  FACTOR_PROP_GET (typ, name, idx) \
  FACTOR_PROP_SET (typ, name, idx)

/* FACTOR_PROP_GET(): macro function for defining a getter function
 * for static factor properties.
 */
#define FACTOR_PROP_GET(typ,name,idx) \
static flt_t *typ ## obj_getparm_ ## name (const factor_t *f) { \
  return float_alloc_with_value(factor_get(f, idx)); }

/* FACTOR_PROP_SET(): macro function for defining a setter function
 * for static factor properties.
 */
#define FACTOR_PROP_SET(typ,name,idx) \
static int typ ## obj_setparm_ ## name (factor_t *f, object_t *val) { \
  if (!OBJECT_IS_NUM(val)) return 0; \
  return factor_set(f, idx, num_get(val)); }

/* factor_t: defined type for the factor structure. */
typedef struct factor factor_t;

/* factor_mean_fn(): return the a first-order mode or moment
 * of a basis element.
 *
 * arguments:
 *  @f: factor structure pointer.
 *  @x: observation input vector.
 *  @p: function output index.
 *  @i: basis element index.
 *
 * returns:
 *  E[ phi^{p}(x | theta(f))_i ]
 */
typedef double (*factor_mean_fn) (const factor_t *f,
                                  const vector_t *x,
                                  const unsigned int p,
                                  const unsigned int i);

/* factor_var_fn(): return the second moment of basis elements.
 *
 * arguments:
 *  @f: factor structure pointer.
 *  @x: observation input vector.
 *  @p: function output index.
 *  @i: first basis element index
 *  @j: second basis element index
 *
 * returns:
 *  E[ phi^{p}(x | theta(f))_i phi^{p}(x | theta(f))_j ]
 */
typedef double (*factor_var_fn) (const factor_t *f,
                                 const vector_t *x,
                                 const unsigned int p,
                                 const unsigned int i,
                                 const unsigned int j);

/* factor_cov_fn(): return the covariance of basis elements.
 *
 * arguments:
 *  @f: factor structure pointer.
 *  @x1: first observation input vector.
 *  @x2: second observation input vector.
 *  @p1: first function output index.
 *  @p2: second function output index.
 *
 * returns:
 *  E[ phi^{p1}(x1 | theta(f)) phi^{p2}(x2 | theta(f)) ]
 */
typedef double (*factor_cov_fn) (const factor_t *f,
                                 const vector_t *x1,
                                 const vector_t *x2,
                                 const unsigned int p1,
                                 const unsigned int p2);

/* factor_diff_mean_fn(): return the gradient of the first moment
 * of a basis element.
 *
 * arguments:
 *  @f: factor structure pointer.
 *  @x: observation input vector.
 *  @p: function output index.
 *  @i: basis element index
 *  @df: vector of gradients.
 *
 * returns:
 *  grad(lambda) E[ phi^{p}(x | theta(f))_i ]
 */
typedef void (*factor_diff_mean_fn) (const factor_t *f,
                                     const vector_t *x,
                                     const unsigned int p,
                                     const unsigned int i,
                                     vector_t *df);

/* factor_diff_var_fn(): return the gradient of the second moment
 * of a basis element.
 *
 * arguments:
 *  @f: factor structure pointer.
 *  @x: observation input vector.
 *  @p: function output index.
 *  @i: first basis element index
 *  @j: second basis element index
 *  @df: vector of gradients.
 *
 * returns:
 *  grad(lambda) E[ phi^{p}(x | theta(f))_i phi^{p}(x | theta(f))_j ]
 */
typedef void (*factor_diff_var_fn) (const factor_t *f,
                                    const vector_t *x,
                                    const unsigned int p,
                                    const unsigned int i,
                                    const unsigned int j,
                                    vector_t *df);

/* factor_meanfield_fn(): perform an assumed-density mean-field update
 * of a factor, given its associated prior factor and a set of required
 * coefficients.
 *
 * if all arguments except the factor structure pointer are null,
 * then the factor shall interpret the function call as a request
 * to prepare itself for receiving data.
 *
 * if all arguments except the factor and its prior are null,
 * then the factor shall interpret the function call as a
 * request to finalize its mean-field update.
 *
 * arguments:
 *  @f: factor structure pointer.
 *  @fp: prior factor structure pointer.
 *  @dat: observed data for updates.
 *  @b: vector of first-order coefficients.
 *  @B: matrix of second-order coefficients.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
typedef int (*factor_meanfield_fn) (factor_t *f, const factor_t *fp,
                                    const datum_t *dat,
                                    vector_t *b,
                                    matrix_t *B);

/* factor_div_fn(): return the kullback-liebler divergence between
 * two factors of the same type, but different parameters.
 *
 * arguments:
 *  @f: first factor structure pointer.
 *  @f2: second factor structure pointer.
 *
 * returns:
 *  KL[ q(theta(f)) || q(theta(f2)) ]
 */
typedef double (*factor_div_fn) (const factor_t *f, const factor_t *f2);

/* factor_init_fn(): initialize a factor structure
 * in a type-specific manner.
 *
 * arguments:
 *  @f: factor structure pointer to initialize.
 *
 * returns:
 *  integer indicating initialization success (1) or failure (0).
 */
typedef int (*factor_init_fn) (factor_t *f);

/* factor_resize_fn(): resize a factor structure.
 *
 * arguments:
 *  @f: factor structure pointer to modify.
 *  @D, @P, @K: new factor sizes.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
typedef int (*factor_resize_fn) (factor_t *f,
                                 const unsigned int D,
                                 const unsigned int P,
                                 const unsigned int K);

/* factor_kernel_fn(): write the kernel code of a factor structure.
 *
 * arguments:
 *  @f: factor structure pointer.
 *  @p0: parameter vector offset.
 *
 * returns:
 *  newly allocated string that contains the kernel code, or NULL
 *  on failure. the calling function is responsible for freeing
 *  the string after use.
 */
typedef char* (*factor_kernel_fn) (const factor_t *f,
                                   const unsigned int p0);

/* factor_set_fn(): assign a variational parameter in a factor.
 *
 * arguments:
 *  @f: factor structure pointer to modify.
 *  @i: parameter index to assign.
 *  @value: parameter value.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
typedef int (*factor_set_fn) (factor_t *f, const unsigned int i,
                              const double value);

/* factor_copy_fn(): copy any extra (e.g. aliased) memory from one
 * factor into another.
 *
 * arguments:
 *  @f: factor structure pointer to access.
 *  @fdup: factor structure pointer to copy into.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
typedef int (*factor_copy_fn) (const factor_t *f, factor_t *fdup);

/* factor_free_fn(): free any extra (e.g. aliased) memory that is
 * associated with a factor.
 *
 * arguments:
 *  @f: factor structure pointer to free.
 */
typedef void (*factor_free_fn) (factor_t *f);

/* FACTOR_EVAL(): macro function for declaring and defining
 * functions conforming to factor_mean_fn() for evaluation.
 */
#define FACTOR_EVAL(name) \
double name ## _eval (const factor_t *f, \
                      const vector_t *x, \
                      const unsigned int p, \
                      const unsigned int i)

/* FACTOR_MEAN(): macro function for declaring and defining
 * functions conforming to factor_mean_fn() for expectations.
 */
#define FACTOR_MEAN(name) \
double name ## _mean (const factor_t *f, \
                      const vector_t *x, \
                      const unsigned int p, \
                      const unsigned int i)

/* FACTOR_VAR(): macro function for declaring and defining
 * functions conforming to factor_var_fn().
 */
#define FACTOR_VAR(name) \
double name ## _var (const factor_t *f, \
                     const vector_t *x, \
                     const unsigned int p, \
                     const unsigned int i, \
                     const unsigned int j)

/* FACTOR_COV(): macro function for declaring and defining
 * functions conforming to factor_cov_fn().
 */
#define FACTOR_COV(name) \
double name ## _cov (const factor_t *f, \
                     const vector_t *x1, const vector_t *x2, \
                     const unsigned int p1, const unsigned int p2)

/* FACTOR_DIFF_MEAN(): macro function for declaring and defining
 * functions conforming to factor_diff_mean_fn().
 */
#define FACTOR_DIFF_MEAN(name) \
void name ## _diff_mean (const factor_t *f, \
                         const vector_t *x, \
                         const unsigned int p, \
                         const unsigned int i, \
                         vector_t *df)

/* FACTOR_DIFF_VAR(): macro function for declaring and defining
 * functions conforming to factor_diff_var_fn().
 */
#define FACTOR_DIFF_VAR(name) \
void name ## _diff_var (const factor_t *f, \
                        const vector_t *x, \
                        const unsigned int p, \
                        const unsigned int i, \
                        const unsigned int j, \
                        vector_t *df)

/* FACTOR_MEANFIELD(): macro function for declaring and defining
 * functions conforming to factor_meanfield_fn().
 */
#define FACTOR_MEANFIELD(name) \
int name ## _meanfield (factor_t *f, const factor_t *fp, \
                        const datum_t *dat, \
                        vector_t *b, \
                        matrix_t *B)

/* macro functions for handling the specific types of mean-field calls.
 */
#define FACTOR_MEANFIELD_INIT  !fp && !dat
#define FACTOR_MEANFIELD_END    fp && !dat

/* FACTOR_DIV(): macro function for declaring and defining
 * functions conforming to factor_div_fn().
 */
#define FACTOR_DIV(name) \
double name ## _div (const factor_t *f, const factor_t *f2)

/* FACTOR_INIT(): macro function for declaring and defining
 * functions conforming to factor_init_fn().
 */
#define FACTOR_INIT(name) \
int name ## _init (factor_t *f)

/* FACTOR_RESIZE(): macro function for declaring and defining
 * functions conforming to factor_resize_fn().
 */
#define FACTOR_RESIZE(name) \
int name ## _resize (factor_t *f, const unsigned int D, \
                     const unsigned int P, const unsigned int K)

/* FACTOR_KERNEL(): macro function for declaring and defining
 * functions conforming to factor_kernel_fn().
 */
#define FACTOR_KERNEL(name) \
char* name ## _kernel (const factor_t *f, const unsigned int p0)

/* FACTOR_SET(): macro function for declaring and defining
 * functions conforming to factor_set_fn().
 */
#define FACTOR_SET(name) \
int name ## _set (factor_t *f, const unsigned int i, const double value)

/* FACTOR_COPY(): macro function for declaring and defining
 * functions conforming to factor_copy_fn().
 */
#define FACTOR_COPY(name) \
int name ## _copy (const factor_t *f, factor_t *fdup)

/* FACTOR_FREE(): macro function for declaring and defining
 * functions conforming to factor_free_fn().
 */
#define FACTOR_FREE(name) \
void name ## _free (factor_t *f)

/* factor_type_t: structure for holding type-specific factor information.
 */
typedef struct {
  /* @base: basic object type information. */
  object_type_t base;

  /* factor type-specific sizes (instance values may differ, see below):
   *  @D: number of dimensions.
   *  @P: number of parameters.
   *  @K: number of weights.
   */
  unsigned int D, P, K;

  /* @parnames: initial factor parameter names. */
  char **parnames;

  /* factor type-specific functions:
   *
   *  expectations:
   *   @eval: value at mode.
   *   @mean: first moment.
   *   @var: second moment.
   *   @cov: covariance.
   *
   *  gradients:
   *   @diff_mean: gradient of the first moment.
   *   @diff_var: gradient of the second moment.
   *
   *  mean-field:
   *   @meanfield: update function.
   *
   *  divergence:
   *   @div: kl-divergence between two factors of the same type.
   *
   *  maintenance hooks:
   *   @init: hook for initialization.
   *   @resize: hook for resize handling.
   *   @kernel: hook for kernel construction.
   *   @set: hook for setting parameter values.
   *   @copy: hook for copying extra memory between factors.
   *   @free: hook for extra functionality during deallocation.
   */
  factor_mean_fn      eval;
  factor_mean_fn      mean;
  factor_var_fn       var;
  factor_cov_fn       cov;
  factor_diff_mean_fn diff_mean;
  factor_diff_var_fn  diff_var;
  factor_meanfield_fn meanfield;
  factor_div_fn       div;
  factor_init_fn      init;
  factor_resize_fn    resize;
  factor_kernel_fn    kernel;
  factor_set_fn       set;
  factor_copy_fn      copy;
  factor_free_fn      free;
}
factor_type_t;

/* struct factor: structure for holding a variational factor.
 *
 * each factor holds information on a set of @M basis elements,
 * corresponding to @M model weights. factors accept inputs of
 * dimensionality @D, and have a set number of variational
 * parameters, given by @P.
 */
struct factor {
  /* @type: factor type information. */
  factor_type_t *type;

  /* factor sizes:
   *  @D: number of dimensions.
   *  @P: number of parameters.
   *  @K: number of weights.
   */
  unsigned int D, P, K;

  /* univariate factors only:
   *  @d: input dimension.
   */
  unsigned int d;

  /* factor flags:
   *  @fixed: whether to fix the factor to its current state.
   */
  unsigned int fixed;

  /* storage of core data:
   *  @inf: fisher information matrix.
   *  @par: parameter vector.
   *  @parnames: parameter name strings.
   */
  matrix_t *inf;
  vector_t *par;
  char **parnames;
};

/* function declarations (factor-obj.c): */

#define factor_alloc(T) \
  (factor_t*) obj_alloc((object_type_t*) T)

int factor_init (factor_t *f);

int factor_copy (const factor_t *f,
                 factor_t *fdup);

void factor_free (factor_t *f);

object_t *factor_add (const factor_t *a, const factor_t *b);

object_t *factor_mul (const factor_t *a, const factor_t *b);

object_t *factor_getprop_dim (const factor_t *f);

object_t *factor_getprop_fixed (const factor_t *f);

int factor_setprop_dim (factor_t *f, object_t *val);

int factor_setprop_fixed (factor_t *f, object_t *val);

object_t *factor_setprop (factor_t *f, object_t *args);

/* function declarations (factor.c): */

int factor_resize (factor_t *f,
                   const unsigned int D,
                   const unsigned int P,
                   const unsigned int K);

unsigned int factor_dims (const factor_t *f);

unsigned int factor_parms (const factor_t *f);

unsigned int factor_weights (const factor_t *f);

char *factor_parname (const factor_t *f, const unsigned int i);

double factor_get_by_name (const factor_t *f, const char *name);

double factor_get (const factor_t *f, const unsigned int i);

void factor_set_fixed (factor_t *f, const unsigned int fixed);

int factor_set (factor_t *f, const unsigned int i, const double value);

double factor_eval (const factor_t *f,
                    const vector_t *x,
                    const unsigned int p,
                    const unsigned int i);

double factor_mean (const factor_t *f,
                    const vector_t *x,
                    const unsigned int p,
                    const unsigned int i);

double factor_var (const factor_t *f,
                   const vector_t *x,
                   const unsigned int p,
                   const unsigned int i,
                   const unsigned int j);

double factor_cov (const factor_t *f,
                   const vector_t *x1,
                   const vector_t *x2,
                   const unsigned int p1,
                   const unsigned int p2);

int factor_diff_mean (const factor_t *f,
                      const vector_t *x,
                      const unsigned int p,
                      const unsigned int i,
                      vector_t *df);

int factor_diff_var (const factor_t *f,
                     const vector_t *x,
                     const unsigned int p,
                     const unsigned int i,
                     const unsigned int j,
                     vector_t *df);

int factor_meanfield (factor_t *f, const factor_t *fp, const datum_t *dat,
                      vector_t *b, matrix_t *B);

double factor_div (const factor_t *f, const factor_t *f2);

char *factor_kernel (const factor_t *f, const unsigned int p0);

/* function declarations (factor/fixed-impulse.c): */

int fixed_impulse_set_location (factor_t *f, const double mu);

/* function declarations (factor/polynomial.c): */

int polynomial_set_order (factor_t *f, const unsigned int order);

/* function declarations (factor/product.c): */

unsigned int product_get_size (const factor_t *f);

factor_t *product_get_factor (const factor_t *f, const unsigned int idx);

int product_add_factor (factor_t *f, const unsigned int d, factor_t *fd);

int product_update (factor_t *f);

/* available factor types: */

extern const factor_type_t *vfl_factor_cosine;
extern const factor_type_t *vfl_factor_decay;
extern const factor_type_t *vfl_factor_impulse;
extern const factor_type_t *vfl_factor_fixed_impulse;
extern const factor_type_t *vfl_factor_polynomial;
extern const factor_type_t *vfl_factor_product;

#endif /* !__VFL_FACTOR_H__ */

