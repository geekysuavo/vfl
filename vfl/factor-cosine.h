
/* ensure once-only inclusion. */
#ifndef __VFL_FACTOR_COSINE_H__
#define __VFL_FACTOR_COSINE_H__

/* function declarations (factor-cosine.c): */

factor_t *factor_cosine (const double mu, const double tau);

double factor_cosine_mean (const factor_t *f,
                           const vector_t *x,
                           const unsigned int i);

double factor_cosine_var (const factor_t *f,
                          const vector_t *x,
                          const unsigned int i,
                          const unsigned int j);

void factor_cosine_diff_mean (const factor_t *f,
                              const vector_t *x,
                              const unsigned int i,
                              vector_t *df);

void factor_cosine_diff_var (const factor_t *f,
                             const vector_t *x,
                             const unsigned int i,
                             const unsigned int j,
                             vector_t *df);

double factor_cosine_div (const factor_t *f, const factor_t *f2);

int factor_cosine_set (factor_t *f, const unsigned int i,
                       const double value);

#endif /* !__VFL_FACTOR_COSINE_H__ */

