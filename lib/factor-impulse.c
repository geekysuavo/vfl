
/* include the factor header. */
#include <vfl/factor.h>

/* define the parameter indices. */
#define P_MU   0
#define P_TAU  1

/* factor_impulse(): allocate a new impulse factor.
 *
 * arguments:
 *  @mu: initial location mean.
 *  @tau: initial location precision.
 *
 * returns:
 *  newly allocated and initialized impulse factor.
 */
factor_t *factor_impulse (const double mu, const double tau) {
  /* allocate a factor without any extra memory. */
  factor_t *f = factor_alloc(0, 1, 2, 1);
  if (!f)
    return NULL;

  /* store the expectation function pointers. */
  f->mean = factor_impulse_mean;
  f->var = factor_impulse_var;

  /* store the gradient function pointers. */
  f->diff_mean = factor_impulse_diff_mean;
  f->diff_var = factor_impulse_diff_var;

  /* store the divergence funciton pointer. */
  f->div = factor_impulse_div;

  /* store the assignment function pointer. */
  f->set = factor_impulse_set;

  /* attempt to set the initial factor parameters. */
  if (!f->set(f, P_MU, mu) || !f->set(f, P_TAU, tau)) {
    factor_free(f);
    return NULL;
  }

  /* return the new cosine factor. */
  return f;
}

/* factor_impulse_mean(): evalute the impulse factor mean.
 *  - see factor_mean_fn() for more information.
 */
double factor_impulse_mean (const factor_t *f,
                            const vector_t *x,
                            const unsigned int i) {
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

/* factor_impulse_var(): evalute the impulse factor variance.
 *  - see factor_var_fn() for more information.
 */
double factor_impulse_var (const factor_t *f,
                           const vector_t *x,
                           const unsigned int i,
                           const unsigned int j) {
  /* call the mean function. */
  return factor_impulse_mean(f, x, i);
}

/* factor_impulse_diff_mean(): evaluate the impulse factor mean gradient.
 *  - see factor_diff_mean_fn() for more information.
 */
void factor_impulse_diff_mean (const factor_t *f,
                               const vector_t *x,
                               const unsigned int i,
                               vector_t *df) {
  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* get the factor parameters. */
  const double mu = vector_get(f->par, P_MU);
  const double tau = vector_get(f->par, P_TAU);

  /* compute the mean value and the shift. */
  const double Gx = factor_impulse_mean(f, x, i);
  const double u = xd - mu;

  /* compute the partial derivatives. */
  const double dmu = Gx * tau * u;
  const double dtau = -0.5 * (u * u) * Gx;

  /* store the partial derivatives. */
  vector_set(df, P_MU, dmu);
  vector_set(df, P_TAU, dtau);
}

/* factor_impulse_diff_var(): evaluate the impulse factor variance gradient.
 *  - see factor_diff_var_fn() for more information.
 */
void factor_impulse_diff_var (const factor_t *f,
                              const vector_t *x,
                              const unsigned int i,
                              const unsigned int j,
                              vector_t *df) {
  /* call the mean gradient function. */
  factor_impulse_diff_mean(f, x, i, df);
}

/* factor_impulse_div(): evaluate the impulse factor divergence.
 *  - see factor_div_fn() for more information.
 */
double factor_impulse_div (const factor_t *f, const factor_t *f2) {
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

/* factor_impulse_set(): store a parameter into a impulse factor.
 *  - see factor_set_fn() for more information.
 */
int factor_impulse_set (factor_t *f, const unsigned int i,
                        const double value) {
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

