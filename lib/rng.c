
/* include the pseudorandom number generator header. */
#include <vfl/rng.h>

/* rng_get(): sample a uniform deviate than spans the values stored
 * by the 'unsigned long long' type.
 *
 * arguments:
 *  @gen: pointer to the generator structure to use for sampling.
 *
 * returns:
 *  new uniformly distributed sample.
 */
static inline unsigned long long rng_get (rng_t *gen) {
  gen->u = gen->u * 2862933555777941757ll + 7046029254386353087ll;
  gen->v ^= gen->v >> 17;
  gen->v ^= gen->v << 31;
  gen->v ^= gen->v >> 8;
  gen->w = 4294957665u * (gen->w & 0xffffffff) + (gen->w >> 32);
  unsigned long long x = gen->u ^ (gen->u << 21);
  x ^= x >> 35;
  x ^= x << 4;
  return (x + gen->v) ^ gen->w;
}

/* --- */

/* rng_alloc(): allocate a new pseudorandom number generator.
 *
 * returns:
 *  newly allocated and initialized generator structure pointer,
 *  seeded either by the value of the environment variable 'RNG_SEED'
 *  or by the default value, 12357.
 */
rng_t *rng_alloc (void) {
  /* declare required variables:
   *  @gen: pointer to the new random number generator structure.
   *  @sstr: environment variable string for reading seed values.
   */
  rng_t *gen;
  char *sstr;

  /* allocate a new structure pointer, or fail. */
  gen = (rng_t*) malloc(sizeof(rng_t));
  if (!gen)
    return NULL;

  /* initialize the random seed. */
  gen->seed = 12357;

  /* if available, read a seed from the environment. */
  sstr = getenv("RNG_SEED");
  if (sstr)
    gen->seed = atoll(sstr);

  /* initialize the core generator variables. */
  gen->v = 4101842887655102017ll;
  gen->u = gen->seed ^ gen->v;
  rng_get(gen);
  gen->v = gen->u;
  rng_get(gen);
  gen->w = gen->v;
  rng_get(gen);

  /* return the new structure pointer. */
  return gen;
}

/* rng_free(): free an allocated pseudorandom number generator.
 *
 * arguments:
 *  @gen: generator structure pointer to free.
 */
void rng_free (rng_t *gen) {
  /* return if the structure pointer is null. */
  if (!gen) return;

  /* free the structure pointer. */
  free(gen);
}

/* rng_uniform(): sample a floating-point uniform deviate in [0,1].
 *
 * arguments:
 *  @gen: pointer to the generator structure to use for sampling.
 *
 * returns:
 *  new uniformly distributed sample.
 */
double rng_uniform (rng_t *gen) {
  /* rescale a sample into the unit interval. */
  return 5.42101086242752217e-20 * rng_get(gen);
}

/* rng_normal(): sample a floating-point standard normal deviate
 * using the Marsaglia polar method. the second deviate produced
 * by the method is discarded.
 *
 * arguments:
 *  @gen: pointer to the generator structure to use for sampling.
 *
 * returns:
 *  new normally distributed sample.
 */
double rng_normal (rng_t *gen) {
  /* declare required variables:
   *  @x1, @x2: independent normal deviates.
   *  @w: length of the arc formed by @x1, @x2.
   */
  double x1, x2, w;

  /* sample from the unit square until the sample
   * lies within the unit circle.
   */
  do {
    x1 = 2.0 * rng_uniform(gen) - 1.0;
    x2 = 2.0 * rng_uniform(gen) - 1.0;
    w = x1 * x1 + x2 * x2;
  } while (w >= 1.0);

  /* compute the scaling constant and return the new sample. */
  w = sqrt(-2.0 * log(w) / w);
  return x1 * w;
}
