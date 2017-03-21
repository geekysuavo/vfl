
/* include the optimizer header. */
#include <vfl/optim.h>

/* optim_fg(): allocate a new full-gradient optimizer.
 *
 * arguments:
 *  @mdl: model structure pointer.
 *
 * returns:
 *  newly allocated and initialized optimizer.
 */
optim_t *optim_fg (model_t *mdl) {
  /* allocate a new optimizer without any extra memory. */
  optim_t *opt = optim_alloc(mdl, 0);
  if (!opt)
    return NULL;

  /* set the function pointers. */
  opt->iterate = fg_iterate;
  opt->execute = fg_execute;

  /* return the new optimizer. */
  return opt;
}

/* fg_iterate(): iteration function for full-gradient optimization.
 *  - see optim_iterate_fn() for more information.
 */
OPTIM_ITERATE (fg) {
  /* gain references to commonly accessed variables. */
  factor_t **factors = opt->mdl->factors;
  factor_t **priors = opt->mdl->priors;
  const unsigned int N = opt->mdl->dat->N;
  const unsigned int M = opt->mdl->M;

  /* declare variables to track the bound between steps and iterations,
   * and a variable to hold step length.
   */
  double bound, bound_prev, bound_init;
  double gamma;

  /* declare variables for holding factor parameters, gradients,
   * and their associated fisher information matrices.
   */
  vector_view_t xa, xb, x, g;
  matrix_view_t Fs;

  /* initialize the model using a full inference. */
  model_infer(opt->mdl);
  bound = bound_init = model_bound(opt->mdl);

  /* loop over each factor in the model. */
  for (unsigned int j = 0; j < M; j++) {
    /* store the current bound. */
    bound_prev = bound;

    /* gain a reference to the current factor parameter count. */
    const unsigned int P = factors[j]->P;
    if (P == 0)
      continue;

    /* configure the parameter and gradient vector views. */
    xa = vector_subvector(opt->xa, 0, P);
    xb = vector_subvector(opt->xb, 0, P);
    x = vector_subvector(opt->x, 0, P);
    g = vector_subvector(opt->g, 0, P);

    /* configure the fisher information martix view. */
    Fs = matrix_submatrix(opt->Fs, 0, 0, P, P);

    /* copy the prior/posterior factor parameters. */
    vector_copy(&xa, factors[j]->par);
    vector_copy(&xb, priors[j]->par);

    /* compute the parameter gradient from all observations. */
    vector_set_zero(&x);
    for (unsigned int i = 0; i < N; i++)
      model_gradient(opt->mdl, i, j, &x);

    /* copy and decompose the fisher information in order to compute
     * the natural gradient.
     */
    matrix_copy(&Fs, factors[j]->inf);
    chol_decomp(&Fs);
    chol_solve(&Fs, &x, &g);

    /* add the natural gradient to the prior. */
    vector_add(&xb, &g);

    /* initialize the step length using the minimum eigenvalue of
     * the fisher information matrix.
     */
    gamma = eigen_minev(factors[j]->inf, &Fs, &g, &x);
    gamma /= opt->l0;

    /* perform a back-tracking line search. */
    unsigned int valid = 0, steps = 0;
    do {
      /* compute the coefficients of the convex combination between
       * current parameters and (prior + nat. grad.).
       */
      const double fa = 1.0 / (gamma + 1.0);
      const double fb = gamma / (gamma + 1.0);

      /* propose a new parameter vector. */
      vector_set_zero(&x);
      blas_daxpy(fa, &xa, &x);
      blas_daxpy(fb, &xb, &x);

      /* attempt to set the proposed parameters. */
      if (model_set_parms(opt->mdl, j, &x)) {
        /* update the model and compute a new bound. */
        model_update(opt->mdl, j);
        bound = model_bound(opt->mdl);

        /* if the bound has increased, accept it as a valid step. */
        if (bound > bound_prev)
          valid = 1;
      }

      /* in case the step was invalid, update the step length and
       * increment the step count.
       */
      gamma *= opt->dl;
      steps++;
    }
    while (!valid && steps < opt->max_steps);

    /* if a valid step was not identified. */
    if (!valid) {
      /* restore the previous parameters and reset the model. */
      model_set_parms(opt->mdl, j, &xa);
      model_update(opt->mdl, j);
      bound = bound_prev;
    }
  }

  /* store the new lower bound into the optimizer. */
  opt->bound = bound;

  /* return whether or not the bound was changed by iteration. */
  return (bound != bound_init);
}

/* fg_execute(): execution function for full-gradient optimization.
 *  - see optim_iterate_fn() for more information.
 */
OPTIM_EXECUTE (fg) {
  /* declare a variable for storing the lower
   * bound from the previous iteration.
   */
  double bound_prev = opt->bound;

  /* loop for the specified number of iterations. */
  for (unsigned int iter = 0; iter < opt->max_iters; iter++) {
    /* store the previous value of the lower bound. */
    bound_prev = opt->bound;

    /* perform an iteration, and break if the bound is unchanged. */
    if (!optim_iterate(opt))
      break;

    /* break if the bound was decreased. */
    if (opt->bound < bound_prev)
      break;
  }

  /* return whether the bound was changed by the final iteration,
   * which could be a sign that optimization is incomplete.
   */
  return (opt->bound > bound_prev);
}

