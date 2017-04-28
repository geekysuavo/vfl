
/* include the factor header. */
#include <vfl/factor.h>

/* polynomial_init(): initialize the polynomial factor structure.
 *  - see factor_init_fn() for more information.
 */
FACTOR_INIT (polynomial) {
  /* indicate that the factor is fixed. */
  f->fixed = 1;

  /* return success. */
  return 1;
}

/* polynomial_mean(): evaluate the polynomial factor mean.
 *  - see factor_mean_fn() for more information.
 */
FACTOR_MEAN (polynomial) {
  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* compute and return the expectation. */
  return pow(xd, i);
}

/* polynomial_var(): evaluate the polynomial factor variance.
 *  - see factor_var_fn() for more information.
 */
FACTOR_VAR (polynomial) {
  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* compute and return the expectation. */
  return pow(xd, i) * pow(xd, j);
}

/* polynomial_cov(): evaluate the polynomial factor covariance.
 *  - see factor_cov_fn() for more information.
 */
FACTOR_COV (polynomial) {
  /* get the input values along the factor dimension. */
  double xd1 = vector_get(x1, f->d);
  double xd2 = vector_get(x2, f->d);

  /* initialize the computation. */
  double cov = 0.0;

  /* loop over the powers of the first input. */
  double xi = 1.0;
  for (unsigned int i = 0; i < f->K; i++) {
    /* loop over powers of the second input. */
    double xj = 1.0;
    for (unsigned int j = 0; j < f->K; j++) {
      /* include the contribution and update
       * the power of the second input.
       */
      cov += xi * xj;
      xj *= xd2;
    }

    /* update the power of the first input. */
    xi *= xd1;
  }

  /* return the computed result. */
  return cov;
}

/* polynomial_set_order(): set the order of a polynomial factor.
 *
 * arguments:
 *  @f: factor structure pointer.
 *  @order: new factor order.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int polynomial_set_order (factor_t *f, const unsigned int order) {
  /* simply resize the factor to match the desired order. */
  return factor_resize(f, f->D, f->P, order + 1);
}

/* polynomial_type: polynomial factor type structure.
 */
static factor_type_t polynomial_type = {
  { /* base: */
    "polynomial",                                /* name      */
    sizeof(factor_t),                            /* size      */
    NULL                                         /* methods   */
  },

  1,                                             /* initial D */
  0,                                             /* initial P */
  1,                                             /* initial K */
  NULL,                                          /* parnames  */
  polynomial_mean,                               /* eval      */
  polynomial_mean,                               /* mean      */
  polynomial_var,                                /* var       */
  polynomial_cov,                                /* cov       */
  NULL,                                          /* diff_mean */
  NULL,                                          /* diff_var  */
  NULL,                                          /* meanfield */
  NULL,                                          /* div       */
  NULL,                                          /* init      */
  NULL,                                          /* resize    */
  NULL,                                          /* kernel    */
  NULL,                                          /* set       */
  NULL,                                          /* copy      */
  NULL                                           /* free      */
};

/* vfl_factor_polynomial: address of the polynomial_type
 * structure.
 */
const factor_type_t *vfl_factor_polynomial = &polynomial_type;

