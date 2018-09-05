
/* include the vfl header. */
#include <vfl/vfl.h>

/* FullGradient: structure for holding full-gradient optimizers.
 */
typedef struct {
  /* optimizer superclass. */
  Optim super;

  /* subclass struct members. */
}
FullGradient;

/* define documentation strings: */

PyDoc_STRVAR(
  FullGradient_doc,
"FullGradient() -> FullGradient object\n"
"\n");

/* FullGradient_iterate(): iteration function for FullGradient.
 *  - see optim_iterate_fn() for more information.
 */
OPTIM_ITERATE (FullGradient) {
  /* gain references to commonly accessed variables. */
  Factor **factors = opt->mdl->factors;
  Factor **priors = opt->mdl->priors;
  const size_t N = opt->mdl->dat->N;
  const size_t M = opt->mdl->M;

  /* declare variables to track the bound between steps and iterations,
   * and a variable to hold step length.
   */
  double bound, bound_prev, bound_init;
  double gamma;

  /* declare variables for holding factor parameters, gradients,
   * and their associated fisher information matrices.
   */
  VectorView xa, xb, x, g;
  MatrixView Fs;

  /* initialize the model using a full inference. */
  model_infer(opt->mdl);
  bound = bound_init = model_bound(opt->mdl);

  /* loop over each factor in the model. */
  for (size_t j = 0; j < M; j++) {
    /* store the current bound. */
    bound_prev = bound;

    /* gain a reference to the current factor parameter count. */
    const size_t P = factors[j]->P;
    if (P == 0 || factors[j]->fixed)
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
    for (size_t i = 0; i < N; i++)
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
    size_t steps = 0;
    int valid = 0;
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

/* FullGradient_execute(): execution function for FullGradient.
 *  - see optim_iterate_fn() for more information.
 */
OPTIM_EXECUTE (FullGradient) {
  /* declare a variable for storing the lower
   * bound from the previous iteration.
   */
  double bound_prev = opt->bound;

  /* loop for the specified number of iterations. */
  for (size_t iter = 0; iter < opt->max_iters; iter++) {
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

/* --- */

/* FullGradient_new(): allocate a new full-gradient optimizer.
 *  - see PyTypeObject.tp_new for details.
 */
VFL_TYPE_NEW (FullGradient) {
  /* allocate a new full-gradient optimizer. */
  FullGradient *self = (FullGradient*) type->tp_alloc(type, 0);
  Optim_reset((Optim*) self);
  if (!self)
    return NULL;

  /* set the function pointers. */
  Optim *opt = (Optim*) self;
  opt->iterate = FullGradient_iterate;
  opt->execute = FullGradient_execute;

  /* return the new object. */
  return (PyObject*) self;
}

/* FullGradient_getset: property definition structure for
 * full-gradient optimizers.
 */
static PyGetSetDef FullGradient_getset[] = {
  { NULL }
};

/* FullGradient_methods: method definition structure for
 * full-gradient optimizers.
 */
static PyMethodDef FullGradient_methods[] = {
  { NULL }
};

/* FullGradient_Type, FullGradient_Type_init() */
VFL_TYPE (FullGradient, Optim, optim)

