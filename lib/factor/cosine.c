
/* include the factor header. */
#include <vfl/factor.h>

/* define the parameter indices. */
#define P_MU   0
#define P_TAU  1

/* factor_cosine(): allocate a new cosine factor.
 *
 * arguments:
 *  @mu: initial frequency mean.
 *  @tau: initial frequency precision.
 *
 * returns:
 *  newly allocated and initialized cosine factor.
 */
factor_t *factor_cosine (const double mu, const double tau) {
  /* allocate a factor without any extra memory. */
  factor_t *f = factor_alloc(0, 1, 2, 2);
  if (!f)
    return NULL;

  /* store the expectation function pointers. */
  f->mean = cosine_mean;
  f->var = cosine_var;

  /* store the gradient function pointers. */
  f->diff_mean = cosine_diff_mean;
  f->diff_var = cosine_diff_var;

  /* store the divergence funciton pointer. */
  f->div = cosine_div;

  /* store the assignment function pointer. */
  f->set = cosine_set;

  /* attempt to set the initial factor parameters. */
  if (!f->set(f, P_MU, mu) || !f->set(f, P_TAU, tau)) {
    factor_free(f);
    return NULL;
  }

  /* return the new cosine factor. */
  return f;
}

/* cosine_mean(): evalute the cosine factor mean.
 *  - see factor_mean_fn() for more information.
 */
FACTOR_MEAN (cosine) {
  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* get the factor parameters. */
  const double mu = vector_get(f->par, P_MU);
  const double tau = vector_get(f->par, P_TAU);

  /* compute and return the expectation. */
  return exp(-0.5 * xd * xd / tau) * cos(mu * xd + M_PI_2 * (double) i);
}

/* cosine_var(): evalute the cosine factor variance.
 *  - see factor_var_fn() for more information.
 */
FACTOR_VAR (cosine) {
  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* compute the sum and difference of the phase offsets. */
  const double zp = M_PI_2 * ((double) i + (double) j);
  const double zm = M_PI_2 * ((double) i - (double) j);

  /* get the factor parameters. */
  const double mu = vector_get(f->par, P_MU);
  const double tau = vector_get(f->par, P_TAU);

  /* compute the expectation of each cosine term. */
  const double ep = exp(-2.0 * xd * xd / tau) * cos(2.0 * mu * xd + zp);
  const double em = cos(zm);

  /* compute and return the expectation. */
  return 0.5 * (ep + em);
}

/* cosine_diff_mean(): evaluate the cosine factor mean gradient.
 *  - see factor_diff_mean_fn() for more information.
 */
FACTOR_DIFF_MEAN (cosine) {
  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* get the factor parameters. */
  const double mu = vector_get(f->par, P_MU);
  const double tau = vector_get(f->par, P_TAU);

  /* compute the trigonometric argument. */
  const double theta = mu * xd + M_PI_2 * (double) i;

  /* compute intermediate quantities. */
  const double E = exp(-0.5 * xd * xd / tau);
  const double C = cos(theta);
  const double S = sin(theta);

  /* compute the partial derivatives. */
  const double dmu = -xd * E * S;
  const double dtau = 0.5 * ((xd * xd) / (tau * tau)) * E * C;

  /* store the partial derivatives. */
  vector_set(df, P_MU, dmu);
  vector_set(df, P_TAU, dtau);
}

/* cosine_diff_var(): evaluate the cosine factor variance gradient.
 *  - see factor_diff_var_fn() for more information.
 */
FACTOR_DIFF_VAR (cosine) {
  /* get twice the input value along the factor dimension. */
  const double xp = 2.0 * vector_get(x, f->d);

  /* get the factor parameters. */
  const double mu = vector_get(f->par, P_MU);
  const double tau = vector_get(f->par, P_TAU);

  /* compute the trigonometric argument. */
  const double theta = mu * xp + M_PI_2 * ((double) i + (double) j);

  /* compute intermediate quantities. */
  const double E = exp(-0.5 * xp * xp / tau);
  const double C = cos(theta);
  const double S = sin(theta);

  /* compute the partial derivatives. */
  const double dmu = -0.5 * xp * E * S;
  const double dtau = 0.25 * ((xp * xp) / (tau * tau)) * E * C;

  /* store the partial derivatives. */
  vector_set(df, P_MU, dmu);
  vector_set(df, P_TAU, dtau);
}

/* cosine_div(): evaluate the cosine factor divergence.
 *  - see factor_div_fn() for more information.
 */
FACTOR_DIV (cosine) {
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

/* cosine_set(): store a parameter into a cosine factor.
 *  - see factor_set_fn() for more information.
 */
FACTOR_SET (cosine) {
  /* determine which parameter is being assigned. */
  switch (i) {
    /* frequency mean: in (-inf, inf) */
    case P_MU:;
      const double mu = value;
      vector_set(f->par, i, mu);

      return 1;

    /* frequency precision: in (0, inf) */
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

