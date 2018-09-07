
/* ensure once-only inclusion. */
#ifndef __VFL_PRODUCT_H__
#define __VFL_PRODUCT_H__

/* include vfl headers. */
#include <vfl/factor.h>

/* Product_Check(): macro to check if a PyObject is a Product.
 */
#define Product_Check(v) (Py_TYPE(v) == &Product_Type)

/* Product_GET_SIZE(): macro to get the size of a Product's
 * internal factor array, without any safety checks.
 */
#define Product_GET_SIZE(v) (((Product*) (v))->F)

/* Product_GET_ITEM(): macro to get a subfactor from a product.
 */
#define Product_GET_ITEM(v,i) (((Product*) (v))->factors[i])

/* Product_SET_ITEM(): macro to store a reference into a product factor.
 */
#define Product_SET_ITEM(v,i,f) \
  { Py_INCREF(f); ((Product*) (v))->factors[i] = ((Factor*) (f)); }

/* Product_Type: globally available product factor type structure.
 */
PyAPI_DATA(PyTypeObject) Product_Type;

/* Product: structure for holding product factors.
 */
typedef struct {
  /* factor superclass. */
  Factor super;

  /* subclass struct members:
   *
   *  product sub-factors:
   *   @factors: array of factors in the product.
   *   @F: number of factors in the product.
   *
   *  mean-field update variables:
   *   @b0: backup vector of coefficients.
   *   @B0: backup matrix of coefficients.
   */
  Factor **factors;
  size_t F;
  Vector *b0;
  Matrix *B0;
}
Product;

/* function declarations (factor/product.c): */

PyObject *product_new_with_size (size_t F, size_t D, size_t P, size_t K);

size_t product_get_size (PyObject *self);

int product_set_item (PyObject *self, size_t i, PyObject *v);

int product_update (PyObject *self);

#endif /* !__VFL_PRODUCT_H__ */

