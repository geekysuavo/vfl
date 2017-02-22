
/* ensure once-only inclusion. */
#ifndef __VFL_FACTOR_PRODUCT_H__
#define __VFL_FACTOR_PRODUCT_H__

/* product_t: structure for holding a product factor.
 */
typedef struct {
  /* @base: core factor structure members.
   */
  factor_t base;

  /* @factors: array of factors in the product.
   * @F: number of factors in the product.
   */
  factor_t **factors;
  unsigned int F;
}
product_t;

/* function declarations (factor/product.c): */

factor_t *factor_product (const unsigned int F, ...);

double factor_product_mean (const factor_t *f,
                            const vector_t *x,
                            const unsigned int i);

double factor_product_var (const factor_t *f,
                           const vector_t *x,
                           const unsigned int i,
                           const unsigned int j);

void factor_product_diff_mean (const factor_t *f,
                               const vector_t *x,
                               const unsigned int i,
                               vector_t *df);

void factor_product_diff_var (const factor_t *f,
                              const vector_t *x,
                              const unsigned int i,
                              const unsigned int j,
                              vector_t *df);

double factor_product_div (const factor_t *f, const factor_t *f2);

int factor_product_set (factor_t *f, const unsigned int i,
                        const double value);

int factor_product_copy (const factor_t *f, factor_t *fdup);

void factor_product_free (factor_t *f);

#endif /* !__VFL_FACTOR_PRODUCT_H__ */

