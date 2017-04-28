
/* ensure once-only inclusion. */
#ifndef __VFL_RNG_H__
#define __VFL_RNG_H__

/* include c library headers. */
#include <stdlib.h>
#include <math.h>

/* include vfl core headers. */
#include <vfl/object.h>

/* rng_t: structure for holding a pseudorandom number generator.
 */
typedef struct {
  /* @base: basic object type information. */
  object_type_t base;

  /* current generator state:
   *  @u, @v, @w: core generator variables.
   */
  unsigned long long u, v, w;

  /* generator parameters:
   *  @seed: initial random seed.
   */
  unsigned long long seed;
}
rng_t;

/* function declarations (rng.c): */

rng_t *rng_alloc (void);

void rng_free (rng_t *gen);

double rng_uniform (rng_t *gen);

double rng_normal (rng_t *gen);

/* available object types: */

extern const object_type_t *vfl_object_rng;

#endif /* !__VFL_RNG_H__ */

