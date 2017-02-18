
/* ensure once-only inclusion. */
#ifndef __VFL_FACTOR_H__
#define __VFL_FACTOR_H__

/* include c headers. */
#include <stdlib.h>
#include <math.h>

/* include application headers. */
#include <vfl/util/specfun.h>
#include <vfl/util/matrix.h>
#include <vfl/util/vector.h>

/* factor_t: defined type for the factor structure. */
typedef struct factor factor_t;

/* factor_mean_fn(): return the first moment of a basis element.
 *
 * arguments:
 *  @f: factor structure pointer.
 *  @x: observation input vector.
 *  @i: basis element index
 *
 * returns:
 *  E[ phi(x | theta(f))_i ]
 */
typedef double (*factor_mean_fn) (const factor_t *f,
                                  const vector_t *x,
                                  const unsigned int i);

/* factor_var_fn(): return the second moment of basis elements.
 *
 * arguments:
 *  @f: factor structure pointer.
 *  @x: observation input vector.
 *  @i: first basis element index
 *  @j: second basis element index
 *
 * returns:
 *  E[ phi(x | theta(f))_i phi(x | theta(f))_j ]
 */
typedef double (*factor_var_fn) (const factor_t *f,
                                 const vector_t *x,
                                 const unsigned int i,
                                 const unsigned int j);

/* factor_diff_mean_fn(): return the gradient of the first moment
 * of a basis element.
 *
 * arguments:
 *  @f: factor structure pointer.
 *  @x: observation input vector.
 *  @i: basis element index
 *  @df: vector of gradients.
 *
 * returns:
 *  grad(lambda) E[ phi(x | theta(f))_i ]
 */
typedef void (*factor_diff_mean_fn) (const factor_t *f,
                                     const vector_t *x,
                                     const unsigned int i,
                                     vector_t *df);

/* factor_diff_var_fn(): return the gradient of the second moment
 * of a basis element.
 *
 * arguments:
 *  @f: factor structure pointer.
 *  @x: observation input vector.
 *  @i: first basis element index
 *  @j: second basis element index
 *  @df: vector of gradients.
 *
 * returns:
 *  grad(lambda) E[ phi(x | theta(f))_i phi(x | theta(f))_j ]
 */
typedef void (*factor_diff_var_fn) (const factor_t *f,
                                    const vector_t *x,
                                    const unsigned int i,
                                    const unsigned int j,
                                    vector_t *df);

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

/* struct factor: structure for holding a variational factor.
 *
 * each factor holds information on a set of @M basis elements,
 * corresponding to @M model weights. factors accept inputs of
 * dimensionality @D, and have a set number of variational
 * parameters, given by @P.
 */
struct factor {
  /* factor sizes:
   *  @bytes: number of bytes allocated to the structure.
   *  @D: number of dimensions.
   *  @P: number of parameters.
   *  @K: number of weights.
   */
  unsigned int bytes, D, P, K;

  /* univariate factors:
   *  @d: input dimension.
   */
  unsigned int d;

  /* expectations:
   *  @mean: first moment.
   *  @var: second moment.
   */
  factor_mean_fn mean;
  factor_var_fn var;

  /* gradients:
   *  @diff_mean: gradient of the first moment.
   *  @diff_var: gradient of the second moment.
   */
  factor_diff_mean_fn diff_mean;
  factor_diff_var_fn diff_var;

  /* @div: kl-divergence between two factors of the same type.
   */
  factor_div_fn div;

  /* maintenance hooks:
   *  @set: hook for setting parameter values.
   *  @copy: hook for copying extra memory between factors.
   *  @free: hook for extra functionality during deallocation.
   */
  factor_set_fn set;
  factor_copy_fn copy;
  factor_free_fn free;

  /* storage of core data:
   *  @inf: fisher information matrix.
   *  @par: parameter vector.
   */
  matrix_t *inf;
  vector_t *par;
};

/* function declarations (factor.c): */

factor_t *factor_alloc (const unsigned int bytes,
                        const unsigned int D,
                        const unsigned int P,
                        const unsigned int K);

factor_t *factor_copy (const factor_t *f);

void factor_free (factor_t *f);

double factor_get (const factor_t *f, const unsigned int i);

int factor_set (factor_t *f, const unsigned int i, const double value);

double factor_mean (const factor_t *f,
                    const vector_t *x,
                    const unsigned int i);

double factor_var (const factor_t *f,
                   const vector_t *x,
                   const unsigned int i,
                   const unsigned int j);

int factor_diff_mean (const factor_t *f,
                      const vector_t *x,
                      const unsigned int i,
                      vector_t *df);

int factor_diff_var (const factor_t *f,
                     const vector_t *x,
                     const unsigned int i,
                     const unsigned int j,
                     vector_t *df);

double factor_div (const factor_t *f, const factor_t *f2);

/* derived factor headers: */
#include <vfl/factor/cosine.h>
#include <vfl/factor/decay.h>
#include <vfl/factor/impulse.h>
#include <vfl/factor/fixed-impulse.h>
#include <vfl/factor/polynomial.h>

#endif /* !__VFL_FACTOR_H__ */

