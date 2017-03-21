
/* ensure once-only inclusion. */
#ifndef __VFL_FACTOR_FIXED_IMPULSE_H__
#define __VFL_FACTOR_FIXED_IMPULSE_H__

/* fixed_impulse_t: structure for holding a fixed impulse factor.
 */
typedef struct {
  /* @base: core factor structure members.
   */
  factor_t base;

  /* @mu: fixed location parameter.
   */
  double mu;
}
fixed_impulse_t;

/* function declarations (factor/fixed-impulse.c): */

factor_t *factor_fixed_impulse (const double mu, const double tau);

FACTOR_MEAN (fixed_impulse);
FACTOR_VAR  (fixed_impulse);

FACTOR_DIFF_MEAN (fixed_impulse);
FACTOR_DIFF_VAR  (fixed_impulse);

FACTOR_DIV (fixed_impulse);
FACTOR_SET (fixed_impulse);

FACTOR_COPY (fixed_impulse);

#endif /* !__VFL_FACTOR_FIXED_IMPULSE_H__ */

