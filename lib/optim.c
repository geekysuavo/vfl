
/* include the optimizer header. */
#include <vfl/optim.h>

/* optim_alloc(): allocate a new optimizer.
 *
 * arguments:
 *  @type: pointer to an optimizer type structure.
 *  @mdl: model structure pointer.
 *
 * returns:
 *  newly allocated and initialized optimizer structure pointer.
 */
optim_t *optim_alloc (const optim_type_t *type, model_t *mdl) {
  /* check the type structure pointer. */
  if (!type)
    return NULL;

  /* return null if the model is null or incapable of inference. */
  if (!mdl || !model_infer(mdl))
    return NULL;

  /* allocate the structure pointer. */
  optim_t *opt = malloc(type->size);
  if (!opt)
    return NULL;

  /* initialize the optimizer type. */
  opt->type = *type;

  /* store the associated model. */
  opt->mdl = mdl;

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
  if (!opt->xa || !opt->xb || !opt->x || !opt->g || !opt->Fs) {
    optim_free(opt);
    return NULL;
  }

  /* initialize the control parameters. */
  opt->max_steps = 10;
  opt->max_iters = 1000;
  opt->l0 = 1.0;
  opt->dl = 0.1;

  /* initialize the lower bound. */
  opt->bound0 = opt->bound = model_bound(mdl);

  /* execute the initialization function, if defined. */
  optim_init_fn init_fn = OPTIM_TYPE(opt)->init;
  if (init_fn && !init_fn(opt)) {
    /* failed to init. free allocated memory and return failure. */
    optim_free(opt);
    return NULL;
  }

  /* return the new optimizer. */
  return opt;
}

/* optim_free(): free an allocated optimizer.
 *
 * arguments:
 *  @opt: optimizer structure pointer to free.
 */
void optim_free (optim_t *opt) {
  /* return if the structure pointer is null. */
  if (!opt)
    return;

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

  /* free the structure pointer. */
  free(opt);
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

