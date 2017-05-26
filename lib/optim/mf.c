
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

  /* determine the maximum number of factors to update. */
  unsigned int M = 0, K = 0;
  const unsigned int Kmax = opt->mdl->dat->N;
  for (unsigned int j = 0; j < opt->mdl->M; j++) {
    const unsigned int Kj = opt->mdl->factors[j]->K;
    if (K + Kj < Kmax) {
      K += Kj;
      M++;
    }
    else
      break;
  }

  /* update each factor in the model. */
  for (unsigned int j = 0; j < M; j++) {
    /* update the factor and, if necessary, re-infer the weights. */
    if (model_meanfield(opt->mdl, j))
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

/* --- */

/* mf_properties: array of accessible mean field
 * optimizer object properties.
 */
static object_property_t mf_properties[] = {
  OPTIM_PROP_BASE,
  { NULL, NULL, NULL }
};

/* mf_methods: array of callable mean field
 * optimizer object methods.
 */
static object_method_t mf_methods[] = {
  OPTIM_METHOD_BASE,
  { NULL, NULL }
};

/* mf_type: optimizer type structure for mean-field training.
 */
static optim_type_t mf_type = {
  { /* base: */
    "mf",                                        /* name    */
    sizeof(optim_t),                             /* size    */

    (object_init_fn) optim_init,                 /* init    */
    NULL,                                        /* copy    */
    (object_free_fn) optim_free,                 /* free    */
    NULL,                                        /* test    */
    NULL,                                        /* cmp     */

    NULL,                                        /* add     */
    NULL,                                        /* sub     */
    NULL,                                        /* mul     */
    NULL,                                        /* div     */
    NULL,                                        /* pow     */

    NULL,                                        /* get     */
    NULL,                                        /* set     */
    mf_properties,                               /* props   */
    mf_methods                                   /* methods */
  },

  NULL,                                          /* init    */
  mf_iterate,                                    /* iterate */
  mf_execute,                                    /* execute */
  NULL                                           /* free    */
};

/* vfl_optim_mf: address of the mf_type structure. */
const optim_type_t *vfl_optim_mf = &mf_type;

