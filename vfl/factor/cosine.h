
/* ensure once-only inclusion. */
#ifndef __VFL_FACTOR_COSINE_H__
#define __VFL_FACTOR_COSINE_H__

/* function declarations (factor/cosine.c): */

factor_t *factor_cosine (const double mu, const double tau);

FACTOR_MEAN (cosine);
FACTOR_VAR  (cosine);

FACTOR_DIFF_MEAN (cosine);
FACTOR_DIFF_VAR  (cosine);

FACTOR_DIV (cosine);
FACTOR_SET (cosine);

#endif /* !__VFL_FACTOR_COSINE_H__ */

