
/* include the factor header. */
#include <vfl/factor.h>

/* define the parameter indices. */
#define P_TAU  0

/* factor_fixed_impulse(): allocate a new fixed impulse factor.
 *
 * arguments:
 *  @mu: (fixed) location mean.
 *  @tau: initial location precision.
 *
 * returns:
 *  newly allocated and initialized fixed impulse factor.
 */
factor_t *factor_fixed_impulse (const double mu, const double tau) {
  /* allocate a factor with extra memory. */
  factor_t *f = factor_alloc(sizeof(fixed_impulse_t), 1, 1, 1);
  if (!f)
    return NULL;

  /* store the expectation function pointers. */
  f->mean = factor_fixed_impulse_mean;
  f->var = factor_fixed_impulse_var;

  /* store the gradient function pointers. */
  f->diff_mean = factor_fixed_impulse_diff_mean;
  f->diff_var = factor_fixed_impulse_diff_var;

  /* store the divergence funciton pointer. */
  f->div = factor_fixed_impulse_div;

  /* store the assignment function pointer. */
  f->set = factor_fixed_impulse_set;
  f->copy = factor_fixed_impulse_copy;

  /* attempt to set the initial factor parameter. */
  if (!f->set(f, P_TAU, tau)) {
    factor_free(f);
    return NULL;
  }

  /* store the extra structure members. */
  fixed_impulse_t *fx = (fixed_impulse_t*) f;
  fx->mu = mu;

  /* return the new cosine factor. */
  return f;
}

/* factor_fixed_impulse_mean(): evalute the fixed impulse factor mean.
 *  - see factor_mean_fn() for more information.
 */
double factor_fixed_impulse_mean (const factor_t *f,
                                  const vector_t *x,
                                  const unsigned int i) {
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

/* factor_fixed_impulse_var(): evalute the fixed impulse factor variance.
 *  - see factor_var_fn() for more information.
 */
double factor_fixed_impulse_var (const factor_t *f,
                                 const vector_t *x,
                                 const unsigned int i,
                                 const unsigned int j) {
  /* call the mean function. */
  return factor_fixed_impulse_mean(f, x, i);
}

/* factor_fixed_impulse_diff_mean(): evaluate the fixed impulse factor
 * mean gradient.
 *  - see factor_diff_mean_fn() for more information.
 */
void factor_fixed_impulse_diff_mean (const factor_t *f,
                                     const vector_t *x,
                                     const unsigned int i,
                                     vector_t *df) {
  /* get the location parameter. */
  fixed_impulse_t *fx = (fixed_impulse_t*) f;
  const double mu = fx->mu;

  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* compute the mean value and the shift. */
  const double Gx = factor_fixed_impulse_mean(f, x, i);
  const double u = xd - mu;

  /* compute and store the partial derivative. */
  const double dtau = -0.5 * (u * u) * Gx;
  vector_set(df, P_TAU, dtau);
}

/* factor_fixed_impulse_diff_var(): evaluate the fixed impulse factor
 * variance gradient.
 *  - see factor_diff_var_fn() for more information.
 */
void factor_fixed_impulse_diff_var (const factor_t *f,
                                    const vector_t *x,
                                    const unsigned int i,
                                    const unsigned int j,
                                    vector_t *df) {
  /* call the mean gradient function. */
  factor_fixed_impulse_diff_mean(f, x, i, df);
}

/* factor_fixed_impulse_div(): evaluate the fixed impulse factor divergence.
 *  - see factor_div_fn() for more information.
 */
double factor_fixed_impulse_div (const factor_t *f, const factor_t *f2) {
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

/* factor_fixed_impulse_set(): store a parameter into a fixed impulse factor.
 *  - see factor_set_fn() for more information.
 */
int factor_fixed_impulse_set (factor_t *f, const unsigned int i,
                              const double value) {
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

/* factor_fixed_impulse_copy(): copy extra information between fixed impulse
 * factors.
 *  - see factor_copy_fn() for more information.
 */
int factor_fixed_impulse_copy (const factor_t *f, factor_t *fdup) {
  /* get the extended structure pointers. */
  fixed_impulse_t *fdupx = (fixed_impulse_t*) fdup;
  fixed_impulse_t *fx = (fixed_impulse_t*) f;

  /* copy the location parameter. */
  fdupx->mu = fx->mu;

  /* return success. */
  return 1;
}

