
/* include the factor header. */
#include <vfl/factor.h>

/* define the parameter indices. */
#define P_ALPHA  0
#define P_BETA   1

/* factor_decay(): allocate a new decay factor.
 *
 * arguments:
 *  @alpha: initial shape parameter of the decay rate.
 *  @beta: initial rate parameter of the decay rate.
 *
 * returns:
 *  newly allocated and initialized decay factor.
 */
factor_t *factor_decay (const double alpha, const double beta) {
  /* allocate a factor without any extra memory. */
  factor_t *f = factor_alloc(0, 1, 2, 1);
  if (!f)
    return NULL;

  /* store the expectation function pointers. */
  f->mean = factor_decay_mean;
  f->var = factor_decay_var;

  /* store the gradient function pointers. */
  f->diff_mean = factor_decay_diff_mean;
  f->diff_var = factor_decay_diff_var;

  /* store the divergence funciton pointer. */
  f->div = factor_decay_div;

  /* store the assignment function pointer. */
  f->set = factor_decay_set;

  /* attempt to set the initial factor parameters. */
  if (!f->set(f, P_ALPHA, alpha) || !f->set(f, P_BETA, beta)) {
    factor_free(f);
    return NULL;
  }

  /* return the new decay factor. */
  return f;
}

/* factor_decay_mean(): evalute the decay factor mean.
 *  - see factor_mean_fn() for more information.
 */
double factor_decay_mean (const factor_t *f,
                          const vector_t *x,
                          const unsigned int i) {
  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* get the factor parameters. */
  const double alpha = vector_get(f->par, P_ALPHA);
  const double beta = vector_get(f->par, P_BETA);

  /* compute and return the expectation. */
  return pow(beta / (beta + xd), alpha);
}

/* factor_decay_var(): evalute the decay factor variance.
 *  - see factor_var_fn() for more information.
 */
double factor_decay_var (const factor_t *f,
                         const vector_t *x,
                         const unsigned int i,
                         const unsigned int j) {
  /* get twice the input value along the factor dimension. */
  const double xp = 2.0 * vector_get(x, f->d);

  /* get the factor parameters. */
  const double alpha = vector_get(f->par, P_ALPHA);
  const double beta = vector_get(f->par, P_BETA);

  /* compute and return the expectation. */
  return pow(beta / (beta + xp), alpha);
}

/* factor_decay_diff_mean(): evaluate the decay factor mean gradient.
 *  - see factor_diff_mean_fn() for more information.
 */
void factor_decay_diff_mean (const factor_t *f,
                             const vector_t *x,
                             const unsigned int i,
                             vector_t *df) {
  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* get the factor parameters. */
  const double alpha = vector_get(f->par, P_ALPHA);
  const double beta = vector_get(f->par, P_BETA);

  /* compute intermediate quantities. */
  const double X = beta / (beta + xd);
  const double Y = (alpha * xd) / (beta * beta);

  /* compute the partial derivatives. */
  const double dalpha = pow(X, alpha) * log(X);
  const double dbeta = Y * pow(X, alpha - 1.0);

  /* store the partial derivatives. */
  vector_set(df, P_ALPHA, dalpha);
  vector_set(df, P_BETA, dbeta);
}

/* factor_decay_diff_var(): evaluate the decay factor variance gradient.
 *  - see factor_diff_var_fn() for more information.
 */
void factor_decay_diff_var (const factor_t *f,
                            const vector_t *x,
                            const unsigned int i,
                            const unsigned int j,
                            vector_t *df) {
  /* get twice the input value along the factor dimension. */
  const double xp = 2.0 * vector_get(x, f->d);

  /* get the factor parameters. */
  const double alpha = vector_get(f->par, P_ALPHA);
  const double beta = vector_get(f->par, P_BETA);

  /* compute intermediate quantities. */
  const double X = beta / (beta + xp);
  const double Y = (alpha * xp) / (beta * beta);

  /* compute the partial derivatives. */
  const double dalpha = pow(X, alpha) * log(X);
  const double dbeta = Y * pow(X, alpha - 1.0);

  /* store the partial derivatives. */
  vector_set(df, P_ALPHA, dalpha);
  vector_set(df, P_BETA, dbeta);
}

/* factor_decay_div(): evaluate the decay factor divergence.
 *  - see factor_div_fn() for more information.
 */
double factor_decay_div (const factor_t *f, const factor_t *f2) {
  /* get the first factor parameters. */
  const double alpha = vector_get(f->par, P_ALPHA);
  const double beta = vector_get(f->par, P_BETA);

  /* get the second factor parameters. */
  const double alpha2 = vector_get(f2->par, P_ALPHA);
  const double beta2 = vector_get(f2->par, P_BETA);

  /* compute the divergence. */
  return alpha  * log(beta)  - lgamma(alpha)
       - alpha2 * log(beta2) + lgamma(alpha2)
       + (alpha - alpha2) * (digamma(alpha) - log(beta))
       + (beta - beta2) * (alpha / beta);
}

/* factor_decay_set(): store a parameter into a decay factor.
 *  - see factor_set_fn() for more information.
 */
int factor_decay_set (factor_t *f, const unsigned int i,
                      const double value) {
  /* determine which parameter is being assigned. */
  switch (i) {
    /* shape parameter: in (0, inf) */
    case P_ALPHA:;
      if (value <= 0.0)
        return 0.0;

      {
        const double alpha = value;
        const double beta = vector_get(f->par, P_BETA);

        vector_set(f->par, i, alpha);

        matrix_set(f->inf, P_ALPHA, P_ALPHA, trigamma(alpha));
        matrix_set(f->inf, P_BETA, P_BETA, alpha / (beta * beta));
      }

      return 1;

    /* rate parameter: in (0, inf) */
    case P_BETA:;
      if (value <= 0.0)
        return 0;

      {
        const double alpha = vector_get(f->par, P_ALPHA);
        const double beta = value;

        vector_set(f->par, i, beta);

        matrix_set(f->inf, P_ALPHA, P_BETA, -1.0 / beta);
        matrix_set(f->inf, P_BETA, P_ALPHA, -1.0 / beta);
        matrix_set(f->inf, P_BETA, P_BETA, alpha / (beta * beta));
      }

      return 1;
  }

  /* invalid parameter index. */
  return 0;
}

