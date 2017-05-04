
/* ensure once-only inclusion. */
#ifndef __VFL_OPTIM_H__
#define __VFL_OPTIM_H__

/* include vfl headers. */
#include <vfl/util/eigen.h>
#include <vfl/model.h>

/* OBJECT_IS_OPTIM(): check if an object is an optimizer.
 */
#define OBJECT_IS_OPTIM(obj) \
  (OBJECT_TYPE(obj)->init == (object_init_fn) optim_init)

/* OPTIM_TYPE(): macro function for casting optimizer structure pointers
 * to their associated type structures.
 */
#define OPTIM_TYPE(s) ((optim_type_t*) (s)->type)

/* OPTIM_PROP_BASE: base set of object properties
 * available to all optimizers.
 */
#define OPTIM_PROP_BASE \
  { "bound", (object_getprop_fn) optim_getprop_bound, NULL }, \
  { "model", \
    (object_getprop_fn) optim_getprop_model, \
    (object_setprop_fn) optim_setprop_model }, \
  { "maxIters", \
    (object_getprop_fn) optim_getprop_maxiters, \
    (object_setprop_fn) optim_setprop_maxiters }, \
  { "maxSteps", \
    (object_getprop_fn) optim_getprop_maxsteps, \
    (object_setprop_fn) optim_setprop_maxsteps }, \
  { "lipschitzInit", \
    (object_getprop_fn) optim_getprop_l0, \
    (object_setprop_fn) optim_setprop_l0 }, \
  { "lipschitzStep", \
    (object_getprop_fn) optim_getprop_dl, \
    (object_setprop_fn) optim_setprop_dl }

/* OPTIM_METHOD_BASE: base set of object methods available
 * to all optimizers.
 */
#define OPTIM_METHOD_BASE \
  { "execute", (object_method_fn) optim_method_execute }

/* optim_t: defined type for the optimizer structure. */
typedef struct optim optim_t;

/* optim_init_fn(): initialize an optimization structure
 * in a type-specific manner.
 *
 * arguments:
 *  @opt: optimizer structure pointer to initialize.
 *
 * returns:
 *  integer indicating initialization success (1) or failure (0).
 */
typedef int (*optim_init_fn) (optim_t *opt);

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

/* OPTIM_INIT(): macro function for declaring and defining
 * functions conforming to optim_init_fn().
 */
#define OPTIM_INIT(name) \
int name ## _init (optim_t *opt)

/* OPTIM_ITERATE(): macro function for declaring and defining
 * functions conforming to optim_iterate_fn() for iteration.
 */
#define OPTIM_ITERATE(name) \
int name ## _iterate (optim_t *opt)

/* OPTIM_EXECUTE(): macro function for declaring and defining
 * functions conforming to optim_iterate_fn() for execution.
 */
#define OPTIM_EXECUTE(name) \
int name ## _execute (optim_t *opt)

/* OPTIM_FREE(): macro function for declaring and defining
 * functions conforming to optim_free_fn().
 */
#define OPTIM_FREE(name) \
void name ## _free (optim_t *opt)

/* optim_type_t: structure for holding type-specific
 * optimizer information.
 */
typedef struct {
  /* @base: basic object type information. */
  object_type_t base;

  /* optimizer type-specific functions:
   *  @init: hook for initialization.
   *  @iterate: hook for iterating on the lower bound.
   *  @execute: hook for running free-run optimization.
   *  @free: hook for freeing extra allocated memory.
   */
  optim_init_fn init;
  optim_iterate_fn iterate;
  optim_iterate_fn execute;
  optim_free_fn free;
}
optim_type_t;

/* struct optim: structure for holding an optimizer, used to
 * learn the variational parameters of a model.
 */
struct optim {
  /* @type: optimizer type information. */
  optim_type_t *type;

  /* @mdl: associated variational feature model. */
  model_t *mdl;

  /* proximal gradient step and endpoints:
   *  @xa: initial point, gamma = 0.
   *  @xb: final point, gamma --> inf.
   *  @x: intermediate point.
   *  @g: step vector.
   */
  vector_t *xa, *xb, *x, *g;

  /* iteration and execution control variables:
   *  @max_steps: maximum number of steps per iteration.
   *  @max_iters: maximum number of total iterations.
   *  @bound0: initial lower bound on allocation.
   *  @bound: current lower bound.
   *  @l0: initial lipschitz constant.
   *  @dl: lipschitz step factor.
   */
  unsigned int max_steps, max_iters;
  double bound0, bound, l0, dl;

  /* temporary structures:
   *  @Fs: spectrally-shifted fisher information matrix.
   */
  matrix_t *Fs;
};

/* function declarations (optim-obj.c): */

#define optim_alloc(T) \
  (optim_t*) obj_alloc((object_type_t*) T)

int optim_init (optim_t *opt);

void optim_free (optim_t *opt);

object_t *optim_getprop_bound (const optim_t *opt);

model_t *optim_getprop_model (const optim_t *opt);

object_t *optim_getprop_maxiters (const optim_t *opt);

object_t *optim_getprop_maxsteps (const optim_t *opt);

object_t *optim_getprop_l0 (const optim_t *opt);

object_t *optim_getprop_dl (const optim_t *opt);

int optim_setprop_model (optim_t *opt, object_t *val);

int optim_setprop_maxiters (optim_t *opt, object_t *val);

int optim_setprop_maxsteps (optim_t *opt, object_t *val);

int optim_setprop_l0 (optim_t *opt, object_t *val);

int optim_setprop_dl (optim_t *opt, object_t *val);

object_t *optim_method_execute (optim_t *opt, object_t *args);

/* function declarations (optim.c): */

int optim_set_model (optim_t *opt, model_t *mdl);

int optim_set_max_steps (optim_t *opt, const unsigned int n);

int optim_set_max_iters (optim_t *opt, const unsigned int n);

int optim_set_lipschitz_init (optim_t *opt, const double l0);

int optim_set_lipschitz_step (optim_t *opt, const double dl);

int optim_iterate (optim_t *opt);

int optim_execute (optim_t *opt);

/* available optimizer types: */

extern const optim_type_t *vfl_optim_fg;
extern const optim_type_t *vfl_optim_mf;

#endif /* !__VFL_OPTIM_H__ */

