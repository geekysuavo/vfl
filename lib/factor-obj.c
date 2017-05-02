
/* include the factor object headers. */
#include <vfl/factor.h>
#include <vfl/base/map.h>
#include <vfl/base/list.h>
#include <vfl/base/float.h>

/* factor_init(): initialize a variational factor.
 *
 * arguments:
 *  @f: factor structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int factor_init (factor_t *f) {
  /* get the factor type information. */
  const factor_type_t *type = FACTOR_TYPE(f);

  /* initialize the factor parameter names table. */
  f->parnames = type->parnames;

  /* set the default factor flags and dimension. */
  f->fixed = 0;
  f->d = 0;

  /* initialize the information matrix and parameter vector. */
  f->inf = NULL;
  f->par = NULL;

  /* execute the factor initialization function, if defined. */
  factor_init_fn init_fn = type->init;
  if (init_fn && !init_fn(f))
    return 0;

  /* initialize the factor sizes from their defaults. */
  if (!factor_resize(f, type->D, type->P, type->K))
    return 0;

  /* return success. */
  return 1;
}

/* factor_copy(): perform a deep copy of an allocated factor.
 *
 * arguments:
 *  @f: source factor structure pointer.
 *  @fdup: destination factor structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int factor_copy (const factor_t *f, factor_t *fdup) {
  /* resize the new factor to match the input factor. */
  if (!factor_resize(fdup, f->D, f->P, f->K))
    return 0;

  /* copy the factor flags and dimension. */
  fdup->fixed = f->fixed;
  fdup->d = f->d;

  /* copy the information matrix and parameter vector. */
  matrix_copy(fdup->inf, f->inf);
  vector_copy(fdup->par, f->par);

  /* if the input factor has a copy function assigned, execute it. */
  factor_copy_fn copy_fn = FACTOR_TYPE(f)->copy;
  if (copy_fn && !copy_fn(f, fdup))
    return 0;

  /* return success. */
  return 1;
}

/* factor_free(): free the contents of a variational factor.
 *
 * arguments:
 *  @f: factor structure pointer to free.
 */
void factor_free (factor_t *f) {
  /* if the factor has a free function assigned, execute it. */
  factor_free_fn free_fn = FACTOR_TYPE(f)->free;
  if (free_fn)
    free_fn(f);

  /* free the information matrix and parameter vector. */
  matrix_free(f->inf);
  vector_free(f->par);
}

/* factor_add(): addition function for variational factors.
 *  - see object_binary_fn() for details.
 */
object_t *factor_add (const factor_t *a, const factor_t *b) {
  /* check the object types. */
  if (!OBJECT_IS_FACTOR(a) || !OBJECT_IS_FACTOR(b))
    return NULL;

  /* allocate a list for the factors. */
  list_t *lst = list_alloc_with_length(2);
  if (!lst)
    return NULL;

  /* store the factors into the list and return. */
  list_set(lst, 0, (object_t*) a);
  list_set(lst, 1, (object_t*) b);
  return (object_t*) lst;
}

/* factor_mul(): multiplication function for variational factors.
 *  - see object_binary_fn() for details.
 */
object_t *factor_mul (const factor_t *a, const factor_t *b) {
  /* check the object types. */
  if (!OBJECT_IS_FACTOR(a) || !OBJECT_IS_FACTOR(b))
    return NULL;

  /* allocate a product factor. */
  factor_t *fp = factor_alloc(vfl_factor_product);
  if (!fp)
    return NULL;

  /* include the first factor. */
  if (FACTOR_TYPE(a) == vfl_factor_product) {
    /* add the factors of the product. */
    for (unsigned int i = 0; i < product_get_size(a); i++) {
      factor_t *fi = product_get_factor(a, i);
      if (!product_add_factor(fp, fi->d, fi))
        return NULL;
    }
  }
  else {
    /* add the first factor. */
    if (!product_add_factor(fp, a->d, (factor_t*) a))
      return NULL;
  }

  /* include the second factor. */
  if (FACTOR_TYPE(b) == vfl_factor_product) {
    /* add the factors of the product. */
    for (unsigned int i = 0; i < product_get_size(b); i++) {
      factor_t *fi = product_get_factor(b, i);
      if (!product_add_factor(fp, fi->d, fi))
        return NULL;
    }
  }
  else {
    /* add the first factor. */
    if (!product_add_factor(fp, b->d, (factor_t*) b))
      return NULL;
  }

  /* return the product factor. */
  return (object_t*) fp;
}

/* factor_getprop_dim(): method for getting factor dimension offsets.
 *  - see object_getprop_fn() for details.
 */
object_t *factor_getprop_dim (const factor_t *f) {
  /* return the dimension index as an integer. */
  return (object_t*) int_alloc_with_value(f->d);
}

/* factor_getprop_fixed(): method for getting factor fixed flags.
 *  - see object_getprop_fn() for details.
 */
object_t *factor_getprop_fixed (const factor_t *f) {
  /* return the fixed flag as an integer. */
  return (object_t*) int_alloc_with_value(f->fixed);
}

/* factor_setprop_dim(): method for setting factor dimension offsets.
 *  - see object_setprop_fn() for details.
 */
int factor_setprop_dim (factor_t *f, object_t *val) {
  /* only admit integer values. */
  if (!OBJECT_IS_INT(val))
    return 0;

  /* check that the dimension index is valid. */
  const long dim = int_get((int_t*) val);
  if (dim < 0)
    return 0;

  /* set the dimension index and return. */
  f->d = dim;
  return 1;
}

/* factor_setprop_fixed(): method for setting factor fixed flags.
 *  - see object_setprop_fn() for details.
 */
int factor_setprop_fixed (factor_t *f, object_t *val) {
  /* only admit integer values. */
  if (!OBJECT_IS_INT(val))
    return 0;

  /* set the fixed flag and return. */
  const long fix = int_get((int_t*) val);
  f->fixed = (fix ? 1 : 0);
  return 1;
}

/* factor_setprop(): method for setting dynamic factor parameters.
 *  - see object_method_fn() for details.
 */
object_t *factor_setprop (factor_t *f, object_t *args) {
  /* loop over the arguments. */
  const size_t n = ((map_t*) args)->len;
  for (size_t i = 0; i < n; i++) {
    /* get the current argument. */
    const char *key = map_key((map_t*) args, i);
    object_t *arg = map_val((map_t*) args, i);

    /* check the argument type. */
    if (!OBJECT_IS_NUM(arg))
      return NULL;

    /* search for the parameter name. */
    for (unsigned int p = 0; p < f->P; p++) {
      /* check if we have a match. */
      if (strcmp(f->parnames[p], key) == 0)
        factor_set(f, p, num_get(arg));
    }
  }

  /* return nothing. */
  VFL_RETURN_NIL;
}

