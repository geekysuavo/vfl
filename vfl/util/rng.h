
/* ensure once-only inclusion. */
#ifndef __VFL_RNG_H__
#define __VFL_RNG_H__

/* include c library headers. */
#include <stdlib.h>
#include <math.h>

/* include vfl headers. */
#include <vfl/lang/object.h>

/* OBJECT_IS_RNG(): check if an object is a pseudorandom number generator.
 */
#define OBJECT_IS_RNG(obj) \
  (OBJECT_TYPE(obj) == vfl_object_rng)

/* rng_t: structure for holding a pseudorandom number generator.
 */
typedef struct {
  /* @type: basic object type information. */
  object_type_t *type;

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

/* function declarations (util/rng.c): */

#define rng_alloc() \
  (rng_t*) obj_alloc(vfl_object_rng)

double rng_uniform (rng_t *gen);

double rng_normal (rng_t *gen);

/* available object types: */

extern const object_type_t *vfl_object_rng;

#endif /* !__VFL_RNG_H__ */

