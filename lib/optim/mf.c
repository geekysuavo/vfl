
/* include the optimizer header. */
#include <vfl/optim.h>

/* mf_iterate(): iteration function for mean-field optimization.
 *  - see optim_iterate_fn() for more information.
 */
OPTIM_ITERATE (mf) {
  /* declare variables to track the bound between factor updates. */
  double bound, bound_init;

  /* initialize the model using a full inference. */
  model_infer(opt->mdl);
  bound = bound_init = model_bound(opt->mdl);

  /* update each factor in the model. */
  const unsigned int M = opt->mdl->M;
  for (unsigned int j = 0; j < M; j++) {
    /* update the factor and re-infer the weights. */
    model_meanfield(opt->mdl, j);
    model_update(opt->mdl, j);

    /* recalculate the bound. */
    bound = model_bound(opt->mdl);
  }

  /* store the new lower bound into the optimizer. */
  opt->bound = bound;

  /* return whether or not the bound was changed by iteration. */
  return (bound != bound_init);
}

/* mf_execute(): execution function for mean-field optimization.
 *  - see optim_iterate_fn() for more information.
 */
OPTIM_EXECUTE (mf) {
  /* store the lower bound from the previous iteration. */
  double bound_prev = opt->bound;

  /* loop for the specified number of iterations. */
  for (unsigned int iter = 0; iter < opt->max_iters; iter++) {
    /* perform an iteration, and break if the bound does not improve. */
    bound_prev = opt->bound;
    if (optim_iterate(opt) || opt->bound < bound_prev)
      break;
  }

  /* return whether the bound was changed by the final iteration. */
  return (opt->bound > bound_prev);
}

/* mf_type: optimizer type structure for mean-field training.
 */
static optim_type_t mf_type = {
  "mf",                                          /* name    */
  sizeof(optim_t),                               /* size    */
  NULL,                                          /* init    */
  mf_iterate,                                    /* iterate */
  mf_execute,                                    /* execute */
  NULL                                           /* free    */
};

/* optim_type_mf: address of the mf_type structure. */
const optim_type_t *optim_type_mf = &mf_type;

