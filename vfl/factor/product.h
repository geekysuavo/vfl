
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

FACTOR_MEAN (product);
FACTOR_VAR  (product);

FACTOR_DIFF_MEAN (product);
FACTOR_DIFF_VAR  (product);

FACTOR_DIV (product);
FACTOR_SET (product);

FACTOR_COPY (product);
FACTOR_FREE (product);

#endif /* !__VFL_FACTOR_PRODUCT_H__ */

