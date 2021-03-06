
/* include the vfl header. */
#include <vfl/vfl.h>

/* Optim_reset(): reset the contents of an optimizer structure.
 *
 * arguments:
 *  @opt: model structure pointer to modify.
 */
void Optim_reset (Optim *opt) {
  /* return if the struct pointer is null. */
  if (!opt)
    return;

  /* initialize the function pointers. */
  opt->init    = NULL;
  opt->iterate = NULL;
  opt->execute = NULL;
  opt->free    = NULL;

  /* initialize the associated model. */
  opt->mdl = NULL;

  /* initialize the iteration vectors. */
  opt->xa = NULL;
  opt->xb = NULL;
  opt->x = NULL;
  opt->g = NULL;

  /* allocate the temporaries. */
  opt->Fs = NULL;

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
int optim_set_model (Optim *opt, Model *mdl) {
  /* check the structure pointers. */
  if (!opt || !mdl)
    return 0;

  /* ensure that the model is capable of inference. */
  if (!model_infer(mdl))
    return 0;

  /* drop the current model. */
  Py_XDECREF(opt->mdl);
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
  size_t pmax = 0;
  for (size_t j = 0; j < mdl->M; j++) {
    const size_t pj = mdl->factors[j]->P;
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
  Py_INCREF(mdl);
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
int optim_set_max_steps (Optim *opt, size_t n) {
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
int optim_set_max_iters (Optim *opt, size_t n) {
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
int optim_set_lipschitz_init (Optim *opt, double l0) {
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
int optim_set_lipschitz_step (Optim *opt, double dl) {
  /* check the input arguments. */
  if (!opt || dl <= 0.0)
    return 0;

  /* set the parameter and return success. */
  opt->dl = dl;
  return 1;
}

/* optim_set_log_iters(): set the iteration frequency of logging
 * optimizer outputs.
 *
 * arguments:
 *  @opt: optimizer structure pointer.
 *  @n: iteration frequency value.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int optim_set_log_iters (Optim *opt, size_t n) {
  /* check the input arguments. */
  if (!opt || n == 0)
    return 0;

  /* set the parameter and return success. */
  opt->log_iters = n;
  return 1;
}

/* optim_set_log_parms(): set the flag that enables or disables
 * logging of optimized parameters.
 *
 * arguments:
 *  @opt: optimizer structure pointer.
 *  @b: parameter logging flag.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int optim_set_log_parms (Optim *opt, int b) {
  /* check the input arguments. */
  if (!opt)
    return 0;

  /* set the parameter and return success. */
  opt->log_parms = (b ? 1 : 0);
  return 1;
}

/* optim_set_log_file(): set the filename string for logging
 * optimizer outputs.
 *
 * arguments:
 *  @opt: optimizer structure pointer.
 *  @fname: log filename string.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int optim_set_log_file (Optim *opt, const char *fname) {
  /* check the input arguments. */
  if (!opt)
    return 0;

  /* close any open file handles. */
  if (opt->log_fh) {
    fclose(opt->log_fh);
    opt->log_fh = NULL;
  }

  /* return if the filename is null. */
  if (!fname)
    return 1;

  /* open a new file handle. */
  opt->log_fh = fopen(fname, "w");
  if (!opt->log_fh)
    return 0;

  /* return success. */
  return 1;
}

/* optim_iterate(): perform a single optimization iteration.
 *  - see optim_iterate_fn() for more information.
 */
int optim_iterate (Optim *opt) {
  /* check the input pointer. */
  if (!opt)
    return 0;

  /* check the function pointer. */
  if (!opt->iterate)
    return 0;

  /* run the iteration function. */
  const int ret = opt->iterate(opt);
  opt->iters++;

  /* check if a log file handle is open. */
  if (opt->log_fh) {
    /* check if the current iteration should be logged. */
    if (opt->log_iters <= 1 || opt->iters % (opt->log_iters - 1) == 0) {
      /* print the basic log information. */
      fprintf(opt->log_fh, "%6zu %16.9le", opt->iters, opt->bound);

      /* check if the parameters should be logged. */
      if (opt->log_parms) {
        /* loop over the model factors. */
        for (size_t j = 0; j < opt->mdl->M; j++) {
          /* get the current factor structure pointer. */
          const Factor *fj = opt->mdl->factors[j];

          /* print each of the factor parameters. */
          for (size_t p = 0; p < fj->P; p++)
            fprintf(opt->log_fh, " %16.9le", factor_get(fj, p));
        }
      }

      /* print a newline. */
      fprintf(opt->log_fh, "\n");
      fflush(opt->log_fh);
    }
  }

  /* return the iteration result. */
  return ret;
}

/* optim_execute(): perform multiple free-run optimization iterations.
 *  - see optim_iterate_fn() for more information.
 */
int optim_execute (Optim *opt) {
  /* check the input pointer. */
  if (!opt)
    return 0;

  /* check the function pointer. */
  if (!opt->execute)
    return 0;

  /* run the execution function. */
  opt->iters = 0;
  const int ret = opt->execute(opt);

  /* return the execution result. */
  return ret;
}

