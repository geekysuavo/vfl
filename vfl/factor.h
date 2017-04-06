
/* ensure once-only inclusion. */
#ifndef __VFL_FACTOR_H__
#define __VFL_FACTOR_H__

/* include c headers. */
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

/* include application headers. */
#include <vfl/util/specfun.h>
#include <vfl/data.h>

/* factor_t: defined type for the factor structure. */
typedef struct factor factor_t;

/* factor_mean_fn(): return the first moment of a basis element.
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
 * arguments:
 *  @f: factor structure pointer.
 *  @fp: prior factor structure pointer.
 *  @dat: dataset structure pointer.
 *  @A: matrix of first-order coefficients.
 *  @B: matrix of second-order coefficients.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
typedef int (*factor_meanfield_fn) (factor_t *f, const factor_t *fp,
                                    const data_t *dat,
                                    const matrix_t *A,
                                    const matrix_t *B);

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

/* FACTOR_MEAN(): macro function for declaring and defining
 * functions conforming to factor_mean_fn().
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
                        const data_t *dat, \
                        const matrix_t *A, \
                        const matrix_t *B)

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

/* FACTOR_TYPE(): macro function for casting factor structure pointers
 * to their associated type structures.
 */
#define FACTOR_TYPE(s) ((factor_type_t*) (s))

/* factor_type_t: structure for holding type-specific factor information.
 */
typedef struct {
  /* basic factor type-specific parameters:
   *  @name: string name of the allocated factor.
   *  @size: number of bytes allocated to the structure pointer.
   */
  const char *name;
  long size;

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
   *   @mean: first moment.
   *   @var: second moment.
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
   *   @set: hook for setting parameter values.
   *   @copy: hook for copying extra memory between factors.
   *   @free: hook for extra functionality during deallocation.
   */
  factor_mean_fn      mean;
  factor_var_fn       var;
  factor_diff_mean_fn diff_mean;
  factor_diff_var_fn  diff_var;
  factor_meanfield_fn meanfield;
  factor_div_fn       div;
  factor_init_fn      init;
  factor_resize_fn    resize;
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
  /* @type: factor type fields. */
  factor_type_t type;

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

/* function declarations (factor.c): */

factor_t *factor_alloc (const factor_type_t *type);

factor_t *factor_copy (const factor_t *f);

void factor_free (factor_t *f);

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

int factor_set (factor_t *f, const unsigned int i, const double value);

double factor_mean (const factor_t *f,
                    const vector_t *x,
                    const unsigned int p,
                    const unsigned int i);

double factor_var (const factor_t *f,
                   const vector_t *x,
                   const unsigned int p,
                   const unsigned int i,
                   const unsigned int j);

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

int factor_meanfield (factor_t *f, const factor_t *fp, const data_t *dat,
                      const matrix_t *A, const matrix_t *B);

double factor_div (const factor_t *f, const factor_t *f2);

/* function declarations (factor/fixed-impulse.c): */

int fixed_impulse_set_location (factor_t *f, const double mu);

/* function declarations (factor/polynomial.c): */

int polynomial_set_order (factor_t *f, const unsigned int order);

/* function declarations (factor/product.c): */

int product_add_factor (factor_t *f, const unsigned int d, factor_t *fd);

/* available factor types: */

extern const factor_type_t *factor_type_cosine;
extern const factor_type_t *factor_type_decay;
extern const factor_type_t *factor_type_impulse;
extern const factor_type_t *factor_type_fixed_impulse;
extern const factor_type_t *factor_type_polynomial;
extern const factor_type_t *factor_type_product;

#endif /* !__VFL_FACTOR_H__ */

