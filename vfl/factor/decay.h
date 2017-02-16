
/* ensure once-only inclusion. */
#ifndef __VFL_FACTOR_DECAY_H__
#define __VFL_FACTOR_DECAY_H__

/* function declarations (factor/decay.c): */

factor_t *factor_decay (const double alpha, const double beta);

double factor_decay_mean (const factor_t *f,
                          const vector_t *x,
                          const unsigned int i);

double factor_decay_var (const factor_t *f,
                         const vector_t *x,
                         const unsigned int i,
                         const unsigned int j);

void factor_decay_diff_mean (const factor_t *f,
                             const vector_t *x,
                             const unsigned int i,
                             vector_t *df);

void factor_decay_diff_var (const factor_t *f,
                            const vector_t *x,
                            const unsigned int i,
                            const unsigned int j,
                            vector_t *df);

double factor_decay_div (const factor_t *f, const factor_t *f2);

int factor_decay_set (factor_t *f, const unsigned int i,
                      const double value);

#endif /* !__VFL_FACTOR_DECAY_H__ */

