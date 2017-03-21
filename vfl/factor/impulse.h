
/* ensure once-only inclusion. */
#ifndef __VFL_FACTOR_IMPULSE_H__
#define __VFL_FACTOR_IMPULSE_H__

/* function declarations (factor/impulse.c): */

factor_t *factor_impulse (const double mu, const double tau);

FACTOR_MEAN (impulse);
FACTOR_VAR  (impulse);

FACTOR_DIFF_MEAN (impulse);
FACTOR_DIFF_VAR  (impulse);

FACTOR_DIV (impulse);
FACTOR_SET (impulse);

#endif /* !__VFL_FACTOR_IMPULSE_H__ */

