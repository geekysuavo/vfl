
/* ensure once-only inclusion. */
#ifndef __VFL_OPTIM_H__
#define __VFL_OPTIM_H__

/* include vfl headers. */
#include <vfl/util/eigen.h>
#include <vfl/model.h>

/* Optim_Check(): macro to check if a PyObject is an Optim.
 */
#define Optim_Check(v) (Py_TYPE(v) == &Optim_Type)

/* Optim_Type: globally available optimizer type structure.
 */
PyAPI_DATA(PyTypeObject) Optim_Type;

/* Optim: defined type for the optimizer structure. */
typedef struct optim Optim;

/* optim_init_fn(): initialize an optimization structure
 * in a type-specific manner.
 *
 * arguments:
 *  @opt: optimizer structure pointer to initialize.
 *
 * returns:
 *  integer indicating initialization success (1) or failure (0).
 */
typedef int (*optim_init_fn) (Optim *opt);

/* optim_iterate_fn(): perform a single iteration of optimization.
 *
 * arguments:
 *  @opt: optimizer structure pointer.
 *
 * returns:
 *  integer indicating whether (1) or not (0) the iteration
 *  changed the model.
 */
typedef int (*optim_iterate_fn) (Optim *opt);

/* optim_free_fn(): free any extra (e.g. aliased) memory that is
 * associated with an optimizer.
 *
 * arguments:
 *  @opt: optimizer structure pointer to free.
 */
typedef void (*optim_free_fn) (Optim *opt);

/* OPTIM_INIT(): macro function for declaring and defining
 * functions conforming to optim_init_fn().
 */
#define OPTIM_INIT(name) \
int name ## _init (Optim *opt)

/* OPTIM_ITERATE(): macro function for declaring and defining
 * functions conforming to optim_iterate_fn() for iteration.
 */
#define OPTIM_ITERATE(name) \
int name ## _iterate (Optim *opt)

/* OPTIM_EXECUTE(): macro function for declaring and defining
 * functions conforming to optim_iterate_fn() for execution.
 */
#define OPTIM_EXECUTE(name) \
int name ## _execute (Optim *opt)

/* OPTIM_FREE(): macro function for declaring and defining
 * functions conforming to optim_free_fn().
 */
#define OPTIM_FREE(name) \
void name ## _free (Optim *opt)

/* struct optim: structure for holding an optimizer, used to
 * learn the variational parameters of a model.
 */
struct optim {
  /* object base. */
  PyObject_HEAD

  /* optimizer specialization functions:
   *  @init: hook for initialization.
   *  @iterate: hook for iterating on the lower bound.
   *  @execute: hook for running free-run optimization.
   *  @free: hook for freeing extra allocated memory.
   */
  optim_init_fn init;
  optim_iterate_fn iterate;
  optim_iterate_fn execute;
  optim_free_fn free;

  /* @mdl: associated variational feature model. */
  Model *mdl;

  /* proximal gradient step and endpoints:
   *  @xa: initial point, gamma = 0.
   *  @xb: final point, gamma --> inf.
   *  @x: intermediate point.
   *  @g: step vector.
   */
  Vector *xa, *xb, *x, *g;

  /* iteration and execution control variables:
   *  @iters: current number of iterations in the execution.
   *  @max_steps: maximum number of steps per iteration.
   *  @max_iters: maximum number of total iterations.
   *  @bound0: initial lower bound on allocation.
   *  @bound: current lower bound.
   *  @l0: initial lipschitz constant.
   *  @dl: lipschitz step factor.
   */
  size_t iters, max_steps, max_iters;
  double bound0, bound, l0, dl;

  /* logging control variables:
   *  @log_iters: frequency of log outputs, in iterations.
   *  @log_parms: whether or not to log factor parameters.
   *  @log_fh: file handle of the optimizer log.
   */
  size_t log_iters;
  int log_parms;
  FILE *log_fh;

  /* temporary structures:
   *  @Fs: spectrally-shifted fisher information matrix.
   */
  Matrix *Fs;
};

/* function declarations (optim-obj.c): */
/*

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

object_t *optim_getprop_logiters (const optim_t *opt);

object_t *optim_getprop_logparms (const optim_t *opt);

int optim_setprop_model (optim_t *opt, object_t *val);

int optim_setprop_maxiters (optim_t *opt, object_t *val);

int optim_setprop_maxsteps (optim_t *opt, object_t *val);

int optim_setprop_l0 (optim_t *opt, object_t *val);

int optim_setprop_dl (optim_t *opt, object_t *val);

int optim_setprop_logiters (optim_t *opt, object_t *val);

int optim_setprop_logparms (optim_t *opt, object_t *val);

int optim_setprop_logfile (optim_t *opt, object_t *val);

object_t *optim_method_execute (optim_t *opt, object_t *args);

*/
/* function declarations (optim.c): */
/*

int optim_set_model (optim_t *opt, model_t *mdl);

int optim_set_max_steps (optim_t *opt, const int n);

int optim_set_max_iters (optim_t *opt, const int n);

int optim_set_lipschitz_init (optim_t *opt, const double l0);

int optim_set_lipschitz_step (optim_t *opt, const double dl);

int optim_set_log_iters (optim_t *opt, const int n);

int optim_set_log_parms (optim_t *opt, const int b);

int optim_set_log_file (optim_t *opt, const char *fname);

int optim_iterate (optim_t *opt);

int optim_execute (optim_t *opt);
*/

#endif /* !__VFL_OPTIM_H__ */

