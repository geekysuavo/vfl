
/* include the factor header. */
#include <vfl/factor.h>

/* define the parameter indices. */
#define P_MU   0
#define P_TAU  1

/* cosine_eval(): evaluate the cosine factor at its mode.
 *  - see factor_mean_fn() for more information.
 */
FACTOR_EVAL (cosine) {
  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* get the factor frequency mean. */
  const double mu = vector_get(f->par, P_MU);

  /* evaluate the factor. */
  return cos(mu * xd + M_PI_2 * (double) i);
}

/* cosine_mean(): evaluate the cosine factor mean.
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

/* cosine_var(): evaluate the cosine factor variance.
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

/* cosine_cov(): evaluate the cosine factor covariance.
 *  - see factor_cov_fn() for more information.
 */
FACTOR_COV (cosine) {
  /* get the factor parameters. */
  const double mu = vector_get(f->par, P_MU);
  const double tau = vector_get(f->par, P_TAU);

  /* compute the difference of the inputs and the phase offset. */
  const double xm = vector_get(x1, f->d) - vector_get(x2, f->d);
  const int zm = (p1 == p2 ? 0 : p1 ? -1 : 1);

  /* compute and return the covariance. */
  return exp(-0.5 * xm * xm / tau) * cos(mu * xm + zm);
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

/* cosine_kernel(): write the kernel code of a cosine factor.
 *  - see factor_kernel_fn() for more information.
 */
FACTOR_KERNEL (cosine) {
  /* define the kernel code format string. */
  const char *fmt = "\
const double xd = x1[%u] - x2[%u];\n\
const double mu = par[%u];\n\
const double tau = par[%u];\n\
const double zd = (p1 == p2 ? 0.0 : p1 ? -%lf : %lf);\n\
cov = exp(-0.5 * xd * xd / tau) * cos(mu * xd + zd);\n\
";

  /* allocate the kernel code string. */
  char *kstr = malloc(strlen(fmt) + 8);
  if (!kstr)
    return NULL;

  /* write the kernel code string. */
  sprintf(kstr, fmt, f->d, f->d,
          p0 + P_MU, p0 + P_TAU,
          M_PI_2, M_PI_2);

  /* return the new string. */
  return kstr;
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

/* cosine_names: table of cosine factor parameter names.
 */
char *cosine_names[] = {
  "mu",
  "tau"
};

/* cosine_type: cosine factor type structure.
 */
static factor_type_t cosine_type = {
  { /* base: */
    "cosine",                                    /* name      */
    sizeof(factor_t),                            /* size      */

    (object_init_fn) factor_init,                /* init      */
    (object_copy_fn) factor_copy,                /* copy      */
    (object_free_fn) factor_free,                /* free      */

    NULL,                                        /* add       */
    NULL,                                        /* sub       */
    NULL,                                        /* mul       */
    NULL,                                        /* div       */

    NULL                                         /* methods   */
  },

  1,                                             /* initial D */
  2,                                             /* initial P */
  2,                                             /* initial K */
  cosine_names,                                  /* parnames  */
  cosine_eval,                                   /* eval      */
  cosine_mean,                                   /* mean      */
  cosine_var,                                    /* var       */
  cosine_cov,                                    /* cov       */
  cosine_diff_mean,                              /* diff_mean */
  cosine_diff_var,                               /* diff_var  */
  NULL,                                          /* meanfield */
  cosine_div,                                    /* div       */
  NULL,                                          /* init      */
  NULL,                                          /* resize    */
  cosine_kernel,                                 /* kernel    */
  cosine_set,                                    /* set       */
  NULL,                                          /* copy      */
  NULL                                           /* free      */
};

/* vfl_factor_cosine: address of the cosine_type structure. */
const factor_type_t *vfl_factor_cosine = &cosine_type;

