
/* include the factor and integer headers. */
#include <vfl/factor.h>
#include <vfl/base/int.h>

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

/* --- */

/* polynomial_getprop_order(): get a polynomial factor order.
 *  - see object_getprop_fn() for details.
 */
static int_t *polynomial_getprop_order (const factor_t *f) {
  /* return the order as an integer. */
  return int_alloc_with_value(f->K - 1);
}

/* polynomial_setprop_order(): set a polynomial factor order.
 *  - see object_setprop_fn() for details.
 */
static int polynomial_setprop_order (factor_t *f, object_t *val) {
  /* only allow integers. */
  if (!OBJECT_IS_INT(val))
    return 0;

  /* only allow non-negative orders. */
  const long ord = int_get((int_t*) val);
  if (ord < 0)
    return 0;

  /* set the order and return. */
  return polynomial_set_order(f, ord);
}

/* polynomial_properties: array of accessible
 * polynomial factor properties.
 */
static object_property_t polynomial_properties[] = {
  FACTOR_PROP_BASE,
  { "order",
    (object_getprop_fn) polynomial_getprop_order,
    (object_setprop_fn) polynomial_setprop_order
  },
  { NULL, NULL, NULL }
};

/* --- */

/* polynomial_methods: array of callable object methods.
 */
static object_method_t polynomial_methods[] = {
  FACTOR_METHOD_BASE,
  { NULL, NULL }
};

/* --- */

/* polynomial_type: polynomial factor type structure.
 */
static factor_type_t polynomial_type = {
  { /* base: */
    "polynomial",                                /* name      */
    sizeof(factor_t),                            /* size      */

    (object_init_fn) factor_init,                /* init      */
    (object_copy_fn) factor_copy,                /* copy      */
    (object_free_fn) factor_free,                /* free      */

    (object_binary_fn) factor_add,               /* add       */
    NULL,                                        /* sub       */
    (object_binary_fn) factor_mul,               /* mul       */
    NULL,                                        /* div       */
    NULL,                                        /* pow       */

    NULL,                                        /* get       */
    NULL,                                        /* set       */
    polynomial_properties,                       /* props     */
    polynomial_methods                           /* methods   */
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

