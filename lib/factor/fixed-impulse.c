
/* include the factor header. */
#include <vfl/factor.h>

/* define the parameter indices. */
#define P_TAU  0

/* fixed_impulse_t: structure for holding a fixed impulse factor.
 */
typedef struct {
  /* @base: core factor structure members. */
  factor_t base;

  /* @mu: fixed location parameter. */
  double mu;
}
fixed_impulse_t;

/* fixed_impulse_init(): initialize the fixed impulse factor structure.
 *  - see factor_init_fn() for more information.
 */
FACTOR_INIT (fixed_impulse) {
  /* initialize the impulse location. */
  return fixed_impulse_set_location(f, 0.0);
}

/* fixed_impulse_mean(): evaluate the fixed impulse factor mean.
 *  - see factor_mean_fn() for more information.
 */
FACTOR_MEAN (fixed_impulse) {
  /* get the location parameter. */
  fixed_impulse_t *fx = (fixed_impulse_t*) f;
  const double mu = fx->mu;

  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* get the factor parameter. */
  const double tau = vector_get(f->par, P_TAU);

  /* compute the normalization and shift. */
  const double u = xd - mu;

  /* compute and return the expectation. */
  return exp(-0.5 * tau * u * u);
}

/* fixed_impulse_var(): evaluate the fixed impulse factor variance.
 *  - see factor_var_fn() for more information.
 */
FACTOR_VAR (fixed_impulse) {
  /* call the mean function. */
  return fixed_impulse_mean(f, x, p, i);
}

/* fixed_impulse_diff_mean(): evaluate the fixed impulse factor
 * mean gradient.
 *  - see factor_diff_mean_fn() for more information.
 */
FACTOR_DIFF_MEAN (fixed_impulse) {
  /* get the location parameter. */
  fixed_impulse_t *fx = (fixed_impulse_t*) f;
  const double mu = fx->mu;

  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* compute the mean value and the shift. */
  const double Gx = fixed_impulse_mean(f, x, p, i);
  const double u = xd - mu;

  /* compute and store the partial derivative. */
  const double dtau = -0.5 * (u * u) * Gx;
  vector_set(df, P_TAU, dtau);
}

/* fixed_impulse_diff_var(): evaluate the fixed impulse factor
 * variance gradient.
 *  - see factor_diff_var_fn() for more information.
 */
FACTOR_DIFF_VAR (fixed_impulse) {
  /* call the mean gradient function. */
  fixed_impulse_diff_mean(f, x, p, i, df);
}

/* fixed_impulse_div(): evaluate the fixed impulse factor divergence.
 *  - see factor_div_fn() for more information.
 */
FACTOR_DIV (fixed_impulse) {
  /* get the first factor parameters. */
  const double tau = vector_get(f->par, P_TAU);
  fixed_impulse_t *fx = (fixed_impulse_t*) f;
  const double mu = fx->mu;

  /* get the second factor parameters. */
  const double tau2 = vector_get(f2->par, P_TAU);
  fixed_impulse_t *f2x = (fixed_impulse_t*) f2;
  const double mu2 = f2x->mu;

  /* compute the divergence. */
  return 0.5 * tau2 * (mu * mu + 1.0 / tau - 2.0 * mu * mu2 + mu2 * mu2)
       - 0.5 * log(tau2 / tau) - 0.5;
}

/* fixed_impulse_set(): store a parameter into a fixed impulse factor.
 *  - see factor_set_fn() for more information.
 */
FACTOR_SET (fixed_impulse) {
  /* determine which parameter is being assigned. */
  switch (i) {
    /* location precision: in (0, inf) */
    case P_TAU:;
      const double tau = value;
      if (tau <= 0.0)
        return 0;

      vector_set(f->par, i, tau);
      matrix_set(f->inf, P_TAU, P_TAU, 0.75 / (tau * tau));

      return 1;
  }

  /* invalid parameter index. */
  return 0;
}

/* fixed_impulse_copy(): copy extra information between fixed impulse
 * factors.
 *  - see factor_copy_fn() for more information.
 */
FACTOR_COPY (fixed_impulse) {
  /* get the extended structure pointers. */
  fixed_impulse_t *fdupx = (fixed_impulse_t*) fdup;
  fixed_impulse_t *fx = (fixed_impulse_t*) f;

  /* copy the location parameter. */
  fdupx->mu = fx->mu;

  /* return success. */
  return 1;
}

/* fixed_impulse_set_location(): set the location of a fixed impulse factor.
 *
 * arguments:
 *  @f: factor structure pointer.
 *  @mu: new factor location.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int fixed_impulse_set_location (factor_t *f, const double mu) {
  /* set the impulse location. */
  fixed_impulse_t *fx = (fixed_impulse_t*) f;
  fx->mu = mu;

  /* return success. */
  return 1;
}

/* fixed_impulse_names: table of fixed impulse factor parameter names.
 */
char *fixed_impulse_names[] = {
  "tau"
};

/* fixed_impulse_type: fixed impulse factor type structure.
 */
static factor_type_t fixed_impulse_type = {
  { /* base: */
    "fixed_impulse",                             /* name      */
    sizeof(fixed_impulse_t),                     /* size      */
    NULL                                         /* methods   */
  },

  1,                                             /* initial D */
  1,                                             /* initial P */
  1,                                             /* initial K */
  fixed_impulse_names,                           /* names     */
  NULL,                                          /* eval      */
  fixed_impulse_mean,                            /* mean      */
  fixed_impulse_var,                             /* var       */
  NULL,                                          /* cov       */
  fixed_impulse_diff_mean,                       /* diff_mean */
  fixed_impulse_diff_var,                        /* diff_var  */
  NULL,                                          /* meanfield */
  fixed_impulse_div,                             /* div       */
  fixed_impulse_init,                            /* init      */
  NULL,                                          /* resize    */
  NULL,                                          /* kernel    */
  fixed_impulse_set,                             /* set       */
  fixed_impulse_copy,                            /* copy      */
  NULL                                           /* free      */
};

/* vfl_factor_fixed_impulse: address of the fixed_impulse_type
 * structure.
 */
const factor_type_t *vfl_factor_fixed_impulse = &fixed_impulse_type;

