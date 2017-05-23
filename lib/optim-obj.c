
/* include the optimizer header. */
#include <vfl/optim.h>

/* include the required object headers. */
#include <vfl/base/map.h>
#include <vfl/base/float.h>
#include <vfl/base/string.h>

/* optim_init(): initialize an optimizer.
 *
 * arguments:
 *  @opt: optimizer structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int optim_init (optim_t *opt) {
  /* get the optimizer type information. */
  const optim_type_t *type = OPTIM_TYPE(opt);

  /* initialize the model field. */
  opt->mdl = NULL;

  /* without a model, iteration vectors and temporaries will be empty. */
  const unsigned int pmax = 0;

  /* allocate the iteration vectors. */
  opt->xa = vector_alloc(pmax);
  opt->xb = vector_alloc(pmax);
  opt->x = vector_alloc(pmax);
  opt->g = vector_alloc(pmax);

  /* allocate the temporaries. */
  opt->Fs = matrix_alloc(pmax, pmax);

  /* check that allocation was successful. */
  if (!opt->xa || !opt->xb || !opt->x || !opt->g || !opt->Fs)
    return 0;

  /* initialize the control parameters. */
  opt->max_steps = 10;
  opt->max_iters = 1000;
  opt->l0 = 1.0;
  opt->dl = 0.1;

  /* initialize the logging parameters. */
  opt->log_iters = 1;
  opt->log_parms = 0;
  opt->log_fh = NULL;

  /* initialize the lower bound. */
  opt->bound0 = opt->bound = -INFINITY;

  /* execute the initialization function, if defined. */
  optim_init_fn init_fn = type->init;
  if (init_fn && !init_fn(opt))
    return 0;

  /* return success. */
  return 1;
}

/* optim_free(): free the contents of an optimizer.
 *
 * arguments:
 *  @opt: optimizer structure pointer to free.
 */
void optim_free (optim_t *opt) {
  /* if the optimizer has a free function assigned, execute it. */
  optim_free_fn free_fn = OPTIM_TYPE(opt)->free;
  if (free_fn)
    free_fn(opt);

  /* release the associated model. */
  obj_release((object_t*) opt->mdl);

  /* close any open file handles. */
  if (opt->log_fh)
    fclose(opt->log_fh);

  /* free the iteration vectors. */
  vector_free(opt->xa);
  vector_free(opt->xb);
  vector_free(opt->x);
  vector_free(opt->g);

  /* free the temporaries. */
  matrix_free(opt->Fs);
}

/* --- */

/* optim_getprop_bound(): method for getting optimizer lower bounds.
 *  - see object_getprop_fn() for details.
 */
object_t *optim_getprop_bound (const optim_t *opt) {
  /* return the lower bound as a float. */
  return (object_t*) float_alloc_with_value(opt->bound);
}

/* optim_getprop_model(): method for getting optimizer models.
 *  - see object_getprop_fn() for details.
 */
model_t *optim_getprop_model (const optim_t *opt) {
  /* return the model from the optimizer. */
  return opt->mdl;
}

/* optim_getprop_maxiters(): method for getting optimizer iteration limits.
 *  - see object_getprop_fn() for details.
 */
object_t *optim_getprop_maxiters (const optim_t *opt) {
  /* return the iteration limit as an integer. */
  return (object_t*) int_alloc_with_value(opt->max_iters);
}

/* optim_getprop_maxsteps(): method for getting optimizer step limits.
 *  - see object_getprop_fn() for details.
 */
object_t *optim_getprop_maxsteps (const optim_t *opt) {
  /* return the step limit as an integer. */
  return (object_t*) int_alloc_with_value(opt->max_steps);
}

/* optim_getprop_l0(): method for getting optimizer initial curvatures.
 *  - see object_getprop_fn() for details.
 */
object_t *optim_getprop_l0 (const optim_t *opt) {
  /* return the property as a float. */
  return (object_t*) float_alloc_with_value(opt->l0);
}

/* optim_getprop_dl(): method for getting optimizer curvature step ratios.
 *  - see object_getprop_fn() for details.
 */
object_t *optim_getprop_dl (const optim_t *opt) {
  /* return the property as a float. */
  return (object_t*) float_alloc_with_value(opt->dl);
}

/* optim_getprop_logiters(): method for getting optimizer log iterations.
 *  - see object_getprop_fn() for details.
 */
object_t *optim_getprop_logiters (const optim_t *opt) {
  /* return the property as an integer. */
  return (object_t*) int_alloc_with_value(opt->log_iters);
}

/* optim_getprop_logparms(): method for getting optimizer
 * parameter logging flags.
 *  - see object_getprop_fn() for details.
 */
object_t *optim_getprop_logparms (const optim_t *opt) {
  /* return the property as an integer. */
  return (object_t*) int_alloc_with_value(opt->log_parms);
}

/* --- */

/* optim_setprop_model(): method for setting optimizer models.
 *  - see object_setprop_fn() for details.
 */
int optim_setprop_model (optim_t *opt, object_t *val) {
  /* ensure the value is a model. */
  if (!OBJECT_IS_MODEL(val))
    return 0;

  /* store the model in the optimizer. */
  return optim_set_model(opt, (model_t*) val);
}

/* optim_setprop_maxiters(): method for setting optimizer iteration limits.
 *  - see object_setprop_fn() for details.
 */
int optim_setprop_maxiters (optim_t *opt, object_t *val) {
  /* admit only integer values. */
  if (!OBJECT_IS_INT(val))
    return 0;

  /* set the value and return. */
  return optim_set_max_iters(opt, int_get((int_t*) val));
}

/* optim_setprop_maxsteps(): method for setting optimizer step limits.
 *  - see object_setprop_fn() for details.
 */
int optim_setprop_maxsteps (optim_t *opt, object_t *val) {
  /* admit only integer values. */
  if (!OBJECT_IS_INT(val))
    return 0;

  /* set the value and return. */
  return optim_set_max_steps(opt, int_get((int_t*) val));
}

/* optim_setprop_l0(): method for setting optimizer initial curvatures.
 *  - see object_setprop_fn() for details.
 */
int optim_setprop_l0 (optim_t *opt, object_t *val) {
  /* admit only integer and float values. */
  if (!OBJECT_IS_NUM(val))
    return 0;

  /* set the value and return. */
  return optim_set_lipschitz_init(opt, num_get(val));
}

/* optim_setprop_dl(): method for setting optimizer curvature step ratios.
 *  - see object_setprop_fn() for details.
 */
int optim_setprop_dl (optim_t *opt, object_t *val) {
  /* admit only integer and float values. */
  if (!OBJECT_IS_NUM(val))
    return 0;

  /* set the value and return. */
  return optim_set_lipschitz_step(opt, num_get(val));
}

/* optim_setprop_logiters(): method for setting optimizer log iterations.
 *  - see object_setprop_fn() for details.
 */
int optim_setprop_logiters (optim_t *opt, object_t *val) {
  /* admit only integer values. */
  if (!OBJECT_IS_INT(val))
    return 0;

  /* set the value and return. */
  return optim_set_log_iters(opt, int_get((int_t*) val));
}

/* optim_setprop_logparms(): method for setting optimizer
 * parameter logging flags.
 *  - see object_setprop_fn() for details.
 */
int optim_setprop_logparms (optim_t *opt, object_t *val) {
  /* admit only integer values. */
  if (!OBJECT_IS_INT(val))
    return 0;

  /* set the value and return. */
  return optim_set_log_parms(opt, int_get((int_t*) val));
}

/* optim_setprop_logfile(): method for setting optimizer log filenames.
 *  - see object_setprop_fn() for details.
 */
int optim_setprop_logfile (optim_t *opt, object_t *val) {
  /* handle the special case of setting the log file to nil. */
  if (OBJECT_IS_NIL(val))
    return optim_set_log_file(opt, NULL);

  /* admit only string values. */
  if (!OBJECT_IS_STRING(val))
    return 0;

  /* set the value and return. */
  return optim_set_log_file(opt, string_get((string_t*) val));
}

/* --- */

/* optim_method_execute(): method for running optimizers.
 *  - see object_method_fn() for details.
 */
object_t *optim_method_execute (optim_t *opt, object_t *args) {
  /* execute the optimization function and return nothing. */
  optim_execute(opt);
  VFL_RETURN_NIL;
}

