
/* include the optimizer header. */
#include <vfl/optim.h>

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
  obj_release((object_t*) opt->mdl);
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
  obj_retain(mdl);
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
int optim_set_max_steps (optim_t *opt, const int n) {
  /* check the input arguments. */
  if (!opt || n <= 0)
    return 0;

  /* set the parameter and return success. */
  opt->max_steps = (unsigned int) n;
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
int optim_set_max_iters (optim_t *opt, const int n) {
  /* check the input arguments. */
  if (!opt || n <= 0)
    return 0;

  /* set the parameter and return success. */
  opt->max_iters = (unsigned int) n;
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
int optim_set_log_iters (optim_t *opt, const int n) {
  /* check the input arguments. */
  if (!opt || n <= 0)
    return 0;

  /* set the parameter and return success. */
  opt->log_iters = (unsigned int) n;
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
int optim_set_log_parms (optim_t *opt, const int b) {
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
int optim_set_log_file (optim_t *opt, const char *fname) {
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
int optim_iterate (optim_t *opt) {
  /* check the input pointer. */
  if (!opt)
    return 0;

  /* check the function pointer. */
  optim_iterate_fn iterate_fn = OPTIM_TYPE(opt)->iterate;
  if (!iterate_fn)
    return 0;

  /* run the iteration function. */
  const int ret = iterate_fn(opt);
  opt->iters++;

  /* check if a log file handle is open. */
  if (opt->log_fh) {
    /* check if the current iteration should be logged. */
    if (opt->log_iters <= 1 || opt->iters % (opt->log_iters - 1) == 0) {
      /* print the basic log information. */
      fprintf(opt->log_fh, "%6u %16.9le", opt->iters, opt->bound);

      /* check if the parameters should be logged. */
      if (opt->log_parms) {
        /* loop over the model factors. */
        for (unsigned int j = 0; j < opt->mdl->M; j++) {
          /* get the current factor structure pointer. */
          const factor_t *fj = opt->mdl->factors[j];

          /* print each of the factor parameters. */
          for (unsigned int p = 0; p < fj->P; p++)
            fprintf(opt->log_fh, " %16.9le", factor_get(fj, p));
        }
      }

      /* print a newline. */
      fprintf(opt->log_fh, "\n");
    }
  }

  /* return the iteration result. */
  return ret;
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
  opt->iters = 0;
  const int ret = execute_fn(opt);

  /* return the execution result. */
  return ret;
}

