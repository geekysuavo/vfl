
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

/* function declarations (factor-fixed-impulse.c): */

factor_t *factor_fixed_impulse (const double mu, const double tau);

double factor_fixed_impulse_mean (const factor_t *f,
                                  const vector_t *x,
                                  const unsigned int i);

double factor_fixed_impulse_var (const factor_t *f,
                                 const vector_t *x,
                                 const unsigned int i,
                                 const unsigned int j);

void factor_fixed_impulse_diff_mean (const factor_t *f,
                                     const vector_t *x,
                                     const unsigned int i,
                                     vector_t *df);

void factor_fixed_impulse_diff_var (const factor_t *f,
                                    const vector_t *x,
                                    const unsigned int i,
                                    const unsigned int j,
                                    vector_t *df);

double factor_fixed_impulse_div (const factor_t *f, const factor_t *f2);

int factor_fixed_impulse_set (factor_t *f, const unsigned int i,
                              const double value);

#endif /* !__VFL_FACTOR_FIXED_IMPULSE_H__ */

