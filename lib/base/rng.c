
/* include the pseudorandom number generator header. */
#include <vfl/base/rng.h>

/* include the required object headers. */
#include <vfl/base/map.h>
#include <vfl/base/int.h>
#include <vfl/base/float.h>

/* rng_get(): sample a uniform deviate than spans the values stored
 * by the 'unsigned long long' type.
 *
 * arguments:
 *  @gen: generator structure pointer.
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

/* rng_reseed(): re-initialize the state of a random number generator
 * with a specified seed number.
 *
 * arguments:
 *  @gen: generator structure pointer.
 *  @seed: new seed value.
 */
static void rng_reseed (rng_t *gen, unsigned long long seed) {
  /* set the provided seed value. */
  gen->seed = seed;

  /* initialize the core generator variables. */
  gen->v = 4101842887655102017ll;
  gen->u = gen->seed ^ gen->v;
  rng_get(gen);
  gen->v = gen->u;
  rng_get(gen);
  gen->w = gen->v;
  rng_get(gen);
}

/* --- */

/* rng_init(): initialize a pseudorandom number generator, seeded
 * either by the value of the environment variable 'RNG_SEED'
 * or by the default value, 12357.
 *
 * arguments:
 *  @gen: generator structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int rng_init (rng_t *gen) {
  /* if available, read a seed from the environment. */
  char *sstr = getenv("RNG_SEED");
  if (sstr)
    rng_reseed(gen, atoll(sstr));
  else
    rng_reseed(gen, 12357);

  /* return success. */
  return 1;
}

/* rng_copy(): copy the contents of a random number generator.
 *
 * arguments:
 *  @gen: source rng structure pointer.
 *  @gendup: destination rng structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int rng_copy (const rng_t *gen, rng_t *gendup) {
  /* copy the generator seed. */
  gendup->seed = gen->seed;

  /* copy the generator state. */
  gendup->u = gen->u;
  gendup->v = gen->v;
  gendup->w = gen->w;

  /* return success. */
  return 1;
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

/* --- */

/* rng_getprop_seed(): get the seed value of a random number generator.
 *  - see object_getprop_fn() for details.
 */
static int_t *rng_getprop_seed (const rng_t *gen) {
  /* return the seed value as an integer. */
  return int_alloc_with_value(gen->seed);
}

/* rng_setprop_seed(): set the seed value of a random number generator.
 *  - see object_setprop_fn() for details.
 */
static int rng_setprop_seed (rng_t *gen, object_t *val) {
  /* admit only integer arguments. */
  if (!OBJECT_IS_INT(val))
    return 0;

  /* re-seed the random number generator. */
  rng_reseed(gen, int_get((int_t*) val));
  return 1;
}

/* rng_properties: array of accessible object properties.
 */
static object_property_t rng_properties[] = {
  { "seed",
    (object_getprop_fn) rng_getprop_seed,
    (object_setprop_fn) rng_setprop_seed
  },
  { NULL, NULL, NULL }
};

/* --- */

/* rng_method_uniform(): sample a uniform random deviate from
 * a random number generator.
 *  - see object_method_fn() for details.
 */
static flt_t *rng_method_uniform (rng_t *gen, map_t *args) {
  /* declare required variables:
   *  @dev: standard uniform random deviate.
   *  @lower: transformation lower bound.
   *  @upper: transformation upper bound.
   */
  double dev, lower, upper;

  /* initialize the variables. */
  dev = rng_uniform(gen);
  lower = 0.0;
  upper = 1.0;

  /* get the lower bound value. */
  object_t *arg = map_get(args, "lower");
  if (arg) {
    if (OBJECT_IS_NUM(arg))
      lower = num_get(arg);
    else
      return NULL;
  }

  /* get the upper bound value. */
  arg = map_get(args, "upper");
  if (arg) {
    if (OBJECT_IS_NUM(arg))
      upper = num_get(arg);
    else
      return NULL;
  }

  /* return the transformed random deviate. */
  return float_alloc_with_value(dev * (upper - lower) + lower);
}

/* rng_method_normal(): sample a normal random deviate from
 * a random number generator.
 *  - see object_method_fn() for details.
 */
static flt_t *rng_method_normal (rng_t *gen, map_t *args) {
  /* declare required variables:
   *  @dev: standard normal random deviate.
   *  @sigma: transformation scaling.
   *  @mu: transformation shift.
   */
  double dev, sigma, mu;

  /* initialize the variables. */
  dev = rng_normal(gen);
  sigma = 1.0;
  mu = 0.0;

  /* get the shift value. */
  object_t *arg = map_get(args, "mu");
  if (arg) {
    if (OBJECT_IS_NUM(arg))
      mu = num_get(arg);
    else
      return NULL;
  }

  /* get the scale value, as standard deviation. */
  arg = map_get(args, "sigma");
  if (arg) {
    if (OBJECT_IS_NUM(arg))
      sigma = num_get(arg);
    else
      return NULL;
  }

  /* get the scale value, as precision. */
  arg = map_get(args, "tau");
  if (arg) {
    if (OBJECT_IS_NUM(arg))
      sigma = 1.0 / sqrt(num_get(arg));
    else
      return NULL;
  }

  /* return the transformed random deviate. */
  return float_alloc_with_value(mu + dev * sigma);
}

/* rng_methods: array of callable object methods.
 */
static object_method_t rng_methods[] = {
  { "uniform", (object_method_fn) rng_method_uniform },
  { "normal",  (object_method_fn) rng_method_normal },
  { NULL, NULL }
};

/* --- */

/* rng_type: random number generator type structure.
 */
static object_type_t rng_type = {
  "rng",                                         /* name      */
  sizeof(rng_t),                                 /* size      */

  (object_init_fn) rng_init,                     /* init      */
  (object_copy_fn) rng_copy,                     /* copy      */
  NULL,                                          /* free      */

  NULL,                                          /* add       */
  NULL,                                          /* sub       */
  NULL,                                          /* mul       */
  NULL,                                          /* div       */
  NULL,                                          /* pow       */

  NULL,                                          /* get       */
  NULL,                                          /* set       */
  rng_properties,                                /* props     */
  rng_methods                                    /* methods   */
};

/* vfl_object_rng: address of the rng_type structure. */
const object_type_t *vfl_object_rng = &rng_type;

