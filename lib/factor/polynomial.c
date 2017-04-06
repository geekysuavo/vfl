
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

/* polynomial_mean(): evalute the polynomial factor mean.
 *  - see factor_mean_fn() for more information.
 */
FACTOR_MEAN (polynomial) {
  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* compute and return the expectation. */
  return pow(xd, i);
}

/* polynomial_var(): evalute the polynomial factor variance.
 *  - see factor_var_fn() for more information.
 */
FACTOR_VAR (polynomial) {
  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* compute and return the expectation. */
  return pow(xd, i) * pow(xd, j);
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
  "polynomial",                                  /* name      */
  sizeof(factor_t),                              /* size      */
  1,                                             /* initial D */
  0,                                             /* initial P */
  1,                                             /* initial K */
  NULL,                                          /* parnames  */
  polynomial_mean,                               /* mean      */
  polynomial_var,                                /* var       */
  NULL,                                          /* diff_mean */
  NULL,                                          /* diff_var  */
  NULL,                                          /* meanfield */
  NULL,                                          /* div       */
  NULL,                                          /* init      */
  NULL,                                          /* resize    */
  NULL,                                          /* set       */
  NULL,                                          /* copy      */
  NULL                                           /* free      */
};

/* factor_type_polynomial: address of the polynomial_type
 * structure.
 */
const factor_type_t *factor_type_polynomial = &polynomial_type;

