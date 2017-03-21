
/* ensure once-only inclusion. */
#ifndef __VFL_FACTOR_DECAY_H__
#define __VFL_FACTOR_DECAY_H__

/* function declarations (factor/decay.c): */

factor_t *factor_decay (const double alpha, const double beta);

FACTOR_MEAN (decay);
FACTOR_VAR  (decay);

FACTOR_DIFF_MEAN (decay);
FACTOR_DIFF_VAR  (decay);

FACTOR_DIV (decay);
FACTOR_SET (decay);

#endif /* !__VFL_FACTOR_DECAY_H__ */

