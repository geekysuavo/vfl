
/* include the factor header. */
#include <vfl/factor.h>

/* factor_polynomial(): allocate a new polynomial factor.
 *
 * arguments:
 *  @order: order of the polynomial.
 *
 * returns:
 *  newly allocated and initialized polynomial factor.
 */
factor_t *factor_polynomial (const unsigned int order) {
  /* allocate a factor without any extra memory. */
  factor_t *f = factor_alloc(0, 1, 0, order + 1);
  if (!f)
    return NULL;

  /* store the expectation function pointers. */
  f->mean = factor_polynomial_mean;
  f->var = factor_polynomial_var;

  /* return the new cosine factor. */
  return f;
}

/* factor_polynomial_mean(): evalute the polynomial factor mean.
 *  - see factor_mean_fn() for more information.
 */
double factor_polynomial_mean (const factor_t *f,
                               const vector_t *x,
                               const unsigned int i) {
  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* compute and return the expectation. */
  return pow(xd, i);
}

/* factor_polynomial_var(): evalute the polynomial factor variance.
 *  - see factor_var_fn() for more information.
 */
double factor_polynomial_var (const factor_t *f,
                              const vector_t *x,
                              const unsigned int i,
                              const unsigned int j) {
  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* compute and return the expectation. */
  return pow(xd, i) * pow(xd, j);
}

