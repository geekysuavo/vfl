
/* include the optimizer header. */
#include <vfl/optim.h>

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

  /* free the iteration vectors. */
  vector_free(opt->xa);
  vector_free(opt->xb);
  vector_free(opt->x);
  vector_free(opt->g);

  /* free the temporaries. */
  matrix_free(opt->Fs);
}

/* optim_set_model(): associate a variational feature model with an
 * optimizer.
 *
 * arguments:
 *  @opt: optimizer structure pointer to modify.
 *  @mdl: model structure pointer to assign.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int optim_set_model (optim_t *opt, model_t *mdl) {
  /* check the structure pointers. */
  if (!opt || !mdl)
    return 0;

  /* check that the model is incapable of inference. */
  if (!model_infer(mdl))
    return 0;

  /* drop the current model. */
  opt->mdl = NULL;

  /* free the iteration vectors. */
  vector_free(opt->xa);
  vector_free(opt->xb);
  vector_free(opt->x);
  vector_free(opt->g);
  opt->xa = NULL;
  opt->xb = NULL;
  opt->x = NULL;
  opt->g = NULL;

  /* free the temporaries. */
  matrix_free(opt->Fs);
  opt->Fs = NULL;

  /* determine the maximum parameter count of the model factors. */
  unsigned int pmax = 0;
  for (unsigned int j = 0; j < mdl->M; j++) {
    const unsigned int pj = mdl->factors[j]->P;
    if (pj > pmax)
      pmax = pj;
  }

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

  /* initialize the lower bound. */
  opt->bound0 = opt->bound = model_bound(mdl);

  /* store the associated model. */
  opt->mdl = mdl;

  /* return success. */
  return 1;
}

/* optim_set_max_steps(): set the maximum number of steps per iteration
 * to perform during optimization.
 *
 * arguments:
 *  @opt: optimizer structure pointer.
 *  @n: maximum number of steps.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int optim_set_max_steps (optim_t *opt, const unsigned int n) {
  /* check the input arguments. */
  if (!opt || n == 0)
    return 0;

  /* set the parameter and return success. */
  opt->max_steps = n;
  return 1;
}

/* optim_set_max_iters(): set the maximum number of iterations
 * to perform during optimization.
 *
 * arguments:
 *  @opt: optimizer structure pointer.
 *  @n: maximum number of iterations.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int optim_set_max_iters (optim_t *opt, const unsigned int n) {
  /* check the input arguments. */
  if (!opt || n == 0)
    return 0;

  /* set the parameter and return success. */
  opt->max_iters = n;
  return 1;
}

/* optim_set_lipschitz_init(): set the initial lipschitz constant
 * to use for each iteration.
 *
 * arguments:
 *  @opt: optimizer structure pointer.
 *  @l0: initial lipschitz constant.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int optim_set_lipschitz_init (optim_t *opt, const double l0) {
  /* check the input arguments. */
  if (!opt || l0 <= 0.0)
    return 0;

  /* set the parameter and return success. */
  opt->l0 = l0;
  return 1;
}

/* optim_set_lipschitz_step(): set the lipschitz constant adjustment
 * factor to use for each step.
 *
 * arguments:
 *  @opt: optimizer structure pointer.
 *  @dl: lipschitz adjustment factor.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int optim_set_lipschitz_step (optim_t *opt, const double dl) {
  /* check the input arguments. */
  if (!opt || dl <= 0.0)
    return 0;

  /* set the parameter and return success. */
  opt->dl = dl;
  return 1;
}

/* optim_iterate(): perform a single optimization iteration.
 *  - see optim_iterate_fn() for more information.
 */
int optim_iterate (optim_t *opt) {
  /* check the input pointer. */
  if (!opt)
    return 0;

  /* check the function pointer. */
  optim_iterate_fn iterate_fn = OPTIM_TYPE(opt)->iterate;
  if (!iterate_fn)
    return 0;

  /* run the iteration function. */
  return iterate_fn(opt);
}

/* optim_execute(): perform multiple free-run optimization iterations.
 *  - see optim_iterate_fn() for more information.
 */
int optim_execute (optim_t *opt) {
  /* check the input pointer. */
  if (!opt)
    return 0;

  /* check the function pointer. */
  optim_iterate_fn execute_fn = OPTIM_TYPE(opt)->execute;
  if (!execute_fn)
    return 0;

  /* run the execution function. */
  return execute_fn(opt);
}

