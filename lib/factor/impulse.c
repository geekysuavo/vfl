
/* include the factor header. */
#include <vfl/factor.h>

/* define the parameter indices. */
#define P_MU   0
#define P_TAU  1

/* impulse_mean(): evalute the impulse factor mean.
 *  - see factor_mean_fn() for more information.
 */
FACTOR_MEAN (impulse) {
  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* get the factor parameters. */
  const double mu = vector_get(f->par, P_MU);
  const double tau = vector_get(f->par, P_TAU);

  /* compute the normalization and shift. */
  const double u = xd - mu;

  /* compute and return the expectation. */
  return exp(-0.5 * tau * u * u);
}

/* impulse_var(): evalute the impulse factor variance.
 *  - see factor_var_fn() for more information.
 */
FACTOR_VAR (impulse) {
  /* call the mean function. */
  return impulse_mean(f, x, p, i);
}

/* impulse_diff_mean(): evaluate the impulse factor mean gradient.
 *  - see factor_diff_mean_fn() for more information.
 */
FACTOR_DIFF_MEAN (impulse) {
  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* get the factor parameters. */
  const double mu = vector_get(f->par, P_MU);
  const double tau = vector_get(f->par, P_TAU);

  /* compute the mean value and the shift. */
  const double Gx = impulse_mean(f, x, p, i);
  const double u = xd - mu;

  /* compute the partial derivatives. */
  const double dmu = Gx * tau * u;
  const double dtau = -0.5 * (u * u) * Gx;

  /* store the partial derivatives. */
  vector_set(df, P_MU, dmu);
  vector_set(df, P_TAU, dtau);
}

/* impulse_diff_var(): evaluate the impulse factor variance gradient.
 *  - see factor_diff_var_fn() for more information.
 */
FACTOR_DIFF_VAR (impulse) {
  /* call the mean gradient function. */
  impulse_diff_mean(f, x, p, i, df);
}

/* impulse_div(): evaluate the impulse factor divergence.
 *  - see factor_div_fn() for more information.
 */
FACTOR_DIV (impulse) {
  /* get the first factor parameters. */
  const double mu = vector_get(f->par, P_MU);
  const double tau = vector_get(f->par, P_TAU);

  /* get the second factor parameters. */
  const double mu2 = vector_get(f2->par, P_MU);
  const double tau2 = vector_get(f2->par, P_TAU);

  /* compute the divergence. */
  return 0.5 * tau2 * (mu * mu + 1.0 / tau - 2.0 * mu * mu2 + mu2 * mu2)
       - 0.5 * log(tau2 / tau) - 0.5;
}

/* impulse_set(): store a parameter into a impulse factor.
 *  - see factor_set_fn() for more information.
 */
FACTOR_SET (impulse) {
  /* determine which parameter is being assigned. */
  switch (i) {
    /* location mean: in (-inf, inf) */
    case P_MU:;
      const double mu = value;
      vector_set(f->par, i, mu);

      return 1;

    /* location precision: in (0, inf) */
    case P_TAU:;
      const double tau = value;
      if (tau <= 0.0)
        return 0;

      vector_set(f->par, i, tau);

      matrix_set(f->inf, P_MU, P_MU, tau);
      matrix_set(f->inf, P_TAU, P_TAU, 0.75 / (tau * tau));

      return 1;
  }

  /* invalid parameter index. */
  return 0;
}

/* impulse_type: impulse factor type structure.
 */
static factor_type_t impulse_type = {
  "impulse",                                     /* name      */
  sizeof(factor_t),                              /* size      */
  1,                                             /* initial D */
  2,                                             /* initial P */
  1,                                             /* initial K */
  impulse_mean,                                  /* mean      */
  impulse_var,                                   /* var       */
  impulse_diff_mean,                             /* diff_mean */
  impulse_diff_var,                              /* diff_var  */
  NULL,                                          /* meanfield */
  impulse_div,                                   /* div       */
  NULL,                                          /* init      */
  impulse_set,                                   /* set       */
  NULL,                                          /* copy      */
  NULL                                           /* free      */
};

/* factor_type_impulse: address of the impulse_type structure. */
const factor_type_t *factor_type_impulse = &impulse_type;

