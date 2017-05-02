
/* include the float header. */
#include <vfl/base/float.h>

/* num_get(): get the value of a scalar numeric object.
 *
 * arguments:
 *  @obj: integer or float object structure pointer.
 *
 * returns:
 *  floating-point value of the object.
 */
static inline double num_get (const object_t *obj) {
  /* return the value. */
  return (OBJECT_IS_INT(obj)
           ? (double) int_get((int_t*) obj)
           : float_get((flt_t*) obj));
}

/* float_get(): get the value of a float object.
 *
 * arguments:
 *  @f: float structure pointer.
 *
 * returns:
 *  value of the float object.
 */
inline double float_get (const flt_t *f) {
  /* return the float value. */
  return (f ? f->val : 0.0);
}

/* float_set(): set the value of a float object.
 *
 * arguments:
 *  @f: float structure pointer.
 *  @val: new value to set.
 */
inline void float_set (flt_t *f, const double val) {
  /* if possible, set the value. */
  if (f)
    f->val = val;
}

/* --- */

/* float_init(): initialize a float.
 *
 * arguments:
 *  @f: float object structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int float_init (flt_t *f) {
  /* initialize the value and return success. */
  f->val = 0.0;
  return 1;
}

/* float_copy(): copy the value of one float to another.
 *
 * arguments:
 *  @f: source float structure pointer.
 *  @fdup: destination float structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int float_copy (const flt_t *f, flt_t *fdup) {
  /* copy the value and return success. */
  fdup->val = f->val;
  return 1;
}

/* float_alloc_with_value(): allocate an float object with
 * a specified value.
 *
 * arguments:
 *  @val: value to store in the new float.
 *
 * returns:
 *  newly allocated and initialized float object.
 */
flt_t *float_alloc_with_value (const double val) {
  /* allocate a new integer. */
  flt_t *f = (flt_t*) obj_alloc(vfl_object_float);
  if (!f)
    return NULL;

  /* set the float value and return. */
  f->val = val;
  return f;
}

/* float_add(): addition function for floats.
 *  - see object_binary_fn() for details.
 */
flt_t *float_add (const object_t *a, const object_t *b) {
  /* if both arguments are scalar numbers, return a result. */
  if (OBJECT_IS_NUM(a) && OBJECT_IS_NUM(b))
    return float_alloc_with_value(num_get(a) + num_get(b));

  /* return no result. */
  return NULL;
}

/* float_sub(): subtraction function for floats.
 *  - see object_binary_fn() for details.
 */
flt_t *float_sub (const object_t *a, const object_t *b) {
  /* if both arguments are scalar numbers, return a result. */
  if (OBJECT_IS_NUM(a) && OBJECT_IS_NUM(b))
    return float_alloc_with_value(num_get(a) - num_get(b));

  /* return no result. */
  return NULL;
}

/* float_mul(): multiplication function for floats.
 *  - see object_binary_fn() for details.
 */
flt_t *float_mul (const object_t *a, const object_t *b) {
  /* if both arguments are scalar numbers, return a result. */
  if (OBJECT_IS_NUM(a) && OBJECT_IS_NUM(b))
    return float_alloc_with_value(num_get(a) * num_get(b));

  /* return no result. */
  return NULL;
}

/* float_div(): division function for floats.
 *  - see object_binary_fn() for details.
 */
flt_t *float_div (const object_t *a, const object_t *b) {
  /* if both arguments are scalar numbers, return a result. */
  if (OBJECT_IS_NUM(a) && OBJECT_IS_NUM(b))
    return float_alloc_with_value(num_get(a) / num_get(b));

  /* return no result. */
  return NULL;
}

/* float_type: float type structure.
 */
static object_type_t float_type = {
  "float",                                       /* name      */
  sizeof(flt_t),                                 /* size      */

  (object_init_fn) float_init,                   /* init      */
  (object_copy_fn) float_copy,                   /* copy      */
  NULL,                                          /* free      */

  (object_binary_fn) float_add,                  /* add       */
  (object_binary_fn) float_sub,                  /* sub       */
  (object_binary_fn) float_mul,                  /* mul       */
  (object_binary_fn) float_div,                  /* div       */

  NULL,                                          /* get       */
  NULL,                                          /* set       */
  NULL,                                          /* props     */
  NULL                                           /* methods   */
};

/* vfl_object_float: address of the float_type structure. */
const object_type_t *vfl_object_float = &float_type;

