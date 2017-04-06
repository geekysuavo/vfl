
/* include the factor header. */
#include <vfl/factor.h>

/* define the parameter indices. */
#define P_ALPHA  0
#define P_BETA   1

/* decay_mean(): evalute the decay factor mean.
 *  - see factor_mean_fn() for more information.
 */
FACTOR_MEAN (decay) {
  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* get the factor parameters. */
  const double alpha = vector_get(f->par, P_ALPHA);
  const double beta = vector_get(f->par, P_BETA);

  /* compute and return the expectation. */
  return pow(beta / (beta + xd), alpha);
}

/* decay_var(): evalute the decay factor variance.
 *  - see factor_var_fn() for more information.
 */
FACTOR_VAR (decay) {
  /* get twice the input value along the factor dimension. */
  const double xp = 2.0 * vector_get(x, f->d);

  /* get the factor parameters. */
  const double alpha = vector_get(f->par, P_ALPHA);
  const double beta = vector_get(f->par, P_BETA);

  /* compute and return the expectation. */
  return pow(beta / (beta + xp), alpha);
}

/* decay_diff_mean(): evaluate the decay factor mean gradient.
 *  - see factor_diff_mean_fn() for more information.
 */
FACTOR_DIFF_MEAN (decay) {
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

/* decay_diff_var(): evaluate the decay factor variance gradient.
 *  - see factor_diff_var_fn() for more information.
 */
FACTOR_DIFF_VAR (decay) {
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

/* decay_div(): evaluate the decay factor divergence.
 *  - see factor_div_fn() for more information.
 */
FACTOR_DIV (decay) {
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

/* decay_set(): store a parameter into a decay factor.
 *  - see factor_set_fn() for more information.
 */
FACTOR_SET (decay) {
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

/* decay_names: table of decay factor parameter names.
 */
char *decay_names[] = {
  "alpha",
  "beta"
};

/* decay_type: decay factor type structure.
 */
static factor_type_t decay_type = {
  "decay",                                       /* name      */
  sizeof(factor_t),                              /* size      */
  1,                                             /* initial D */
  2,                                             /* initial P */
  1,                                             /* initial K */
  decay_names,                                   /* parnames  */
  decay_mean,                                    /* mean      */
  decay_var,                                     /* var       */
  decay_diff_mean,                               /* diff_mean */
  decay_diff_var,                                /* diff_var  */
  NULL,                                          /* meanfield */
  decay_div,                                     /* div       */
  NULL,                                          /* init      */
  NULL,                                          /* resize    */
  decay_set,                                     /* set       */
  NULL,                                          /* copy      */
  NULL                                           /* free      */
};

/* factor_type_decay: address of the decay_type structure. */
const factor_type_t *factor_type_decay = &decay_type;

