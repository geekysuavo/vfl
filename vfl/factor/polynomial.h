
/* ensure once-only inclusion. */
#ifndef __VFL_FACTOR_POLYNOMIAL_H__
#define __VFL_FACTOR_POLYNOMIAL_H__

/* function declarations (factor/polynomial.c): */

factor_t *factor_polynomial (const unsigned int order);

FACTOR_MEAN (polynomial);
FACTOR_VAR  (polynomial);

#endif /* !__VFL_FACTOR_POLYNOMIAL_H__ */

