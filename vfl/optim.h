
/* ensure once-only inclusion. */
#ifndef __VFL_OPTIM_H__
#define __VFL_OPTIM_H__

/* include the model and eigenvalue headers. */
#include <vfl/model.h>
#include <vfl/util/eigen.h>

/* optim_t: defined type for the optimizer structure. */
typedef struct optim optim_t;

/* optim_iterate_fn(): perform a single iteration of optimization.
 *
 * arguments:
 *  @opt: optimizer structure pointer.
 *
 * returns:
 *  integer indicating whether (1) or not (0) the iteration
 *  changed the model.
 */
typedef int (*optim_iterate_fn) (optim_t *opt);

/* optim_free_fn(): free any extra (e.g. aliased) memory that is
 * associated with an optimizer.
 *
 * arguments:
 *  @opt: optimizer structure pointer to free.
 */
typedef void (*optim_free_fn) (optim_t *opt);

/* struct optim: structure for holding an optimizer, used to
 * learn the variational parameters of a model.
 */
struct optim {
  /* core optimizer information:
   *  @bytes: number of bytes allocated to the structure.
   *  @mdl: associated variational feature model.
   */
  unsigned int bytes;
  model_t *mdl;

  /* function pointers:
   *  @iterate: hook for iterating on the lower bound.
   *  @free: hook for freeing extra allocated memory.
   */
  optim_iterate_fn iterate;
  optim_free_fn free;

  /* proximal gradient step and endpoints:
   *  @xa: initial point, gamma = 0.
   *  @xb: final point, gamma --> inf.
   *  @x: intermediate point.
   *  @g: step vector.
   *  @l0: initial lipschitz constant.
   *  @dl: lipschitz step factor.
   */
  vector_t *xa, *xb, *x, *g;
  double l0, dl;

  /* temporary structures:
   *  @Fs: spectrally-shifted fisher information matrix.
   */
  matrix_t *Fs;
};

/* function declarations (optim.c): */

optim_t *optim_alloc (model_t *mdl, const unsigned int bytes);

void optim_free (optim_t *opt);

/* derived optimizer headers. */
#include <vfl/optim/fg.h>

#endif /* !__VFL_OPTIM_H__ */

