
/* ensure once-only inclusion. */
#ifndef __VFL_FACTOR_POLYNOMIAL_H__
#define __VFL_FACTOR_POLYNOMIAL_H__

/* function declarations (factor/polynomial.c): */

factor_t *factor_polynomial (const unsigned int order);

double factor_polynomial_mean (const factor_t *f,
                               const vector_t *x,
                               const unsigned int i);

double factor_polynomial_var (const factor_t *f,
                              const vector_t *x,
                              const unsigned int i,
                              const unsigned int j);

#endif /* !__VFL_FACTOR_POLYNOMIAL_H__ */

