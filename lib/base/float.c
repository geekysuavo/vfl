
/* include the required object headers. */
#include <vfl/base/int.h>
#include <vfl/base/float.h>
#include <vfl/base/string.h>

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

/* num_get(): get the value of a scalar numeric object.
 *
 * arguments:
 *  @num: integer or float object structure pointer.
 *
 * returns:
 *  floating-point value of the object.
 */
double num_get (const object_t *num) {
  /* return the value. */
  return (OBJECT_IS_INT(num) ? (double) int_get((int_t*) num)
                             : float_get((flt_t*) num));
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
  /* allocate a new float. */
  flt_t *f = float_alloc();
  if (!f)
    return NULL;

  /* set the float value and return. */
  f->val = val;
  return f;
}

/* float_test(): assertion function for floats.
 *  - see object_test_fn() for details.
 */
int float_test (const flt_t *obj) {
  /* return true for nonzero floats. */
  return (obj->val ? 1 : 0);
}

/* float_cmp(): comparison function for floats.
 *  - see object_comp_fn() for details.
 */
int float_cmp (const object_t *a, const object_t *b) {
  /* if both arguments are scalar numbers, return a result. */
  if (OBJECT_IS_NUM(a) && OBJECT_IS_NUM(b)) {
    const double aval = num_get(a);
    const double bval = num_get(b);
    return (aval < bval ? -1 : aval > bval ? 1 : 0);
  }

  /* return no result. */
  return OBJECT_CMP_ERR;
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

/* float_pow(): exponentiation function for floats.
 *  - see object_binary_fn() for details.
 */
flt_t *float_pow (const object_t *a, const object_t *b) {
  /* if both arguments are scalar numbers, return a result. */
  if (OBJECT_IS_NUM(a) && OBJECT_IS_NUM(b))
    return float_alloc_with_value(pow(num_get(a), num_get(b)));

  /* return no result. */
  return NULL;
}

/* float_setprop_value(): set the value of a float.
 *  - see object_setprop_fn() for details.
 */
static int float_setprop_value (flt_t *f, object_t *val) {
  /* accept integers, floats, and strings. */
  if (OBJECT_IS_INT(val)) {
    /* cast the integer to a float. */
    f->val = (double) int_get((int_t*) val);
    return 1;
  }
  else if (OBJECT_IS_FLOAT(val)) {
    /* set the value from the float. */
    f->val = float_get((flt_t*) val);
    return 1;
  }
  else if (OBJECT_IS_STRING(val)) {
    /* cast the string to a float. */
    f->val = atof(string_get((string_t*) val));
    return 1;
  }

  /* invalid value type. */
  return 0;
}

/* float_properties: array of accessible object properties.
 */
static object_property_t float_properties[] = {
  { "value", NULL, (object_setprop_fn) float_setprop_value },
  { "val",   NULL, (object_setprop_fn) float_setprop_value },
  { "v",     NULL, (object_setprop_fn) float_setprop_value },
  { NULL, NULL, NULL }
};

/* --- */

/* float_type: float type structure.
 */
static object_type_t float_type = {
  "float",                                       /* name      */
  sizeof(flt_t),                                 /* size      */

  (object_init_fn) float_init,                   /* init      */
  (object_copy_fn) float_copy,                   /* copy      */
  NULL,                                          /* free      */
  (object_test_fn) float_test,                   /* test      */
  (object_comp_fn) float_cmp,                    /* cmp       */

  (object_binary_fn) float_add,                  /* add       */
  (object_binary_fn) float_sub,                  /* sub       */
  (object_binary_fn) float_mul,                  /* mul       */
  (object_binary_fn) float_div,                  /* div       */
  (object_binary_fn) float_pow,                  /* pow       */

  NULL,                                          /* get       */
  NULL,                                          /* set       */
  float_properties,                              /* props     */
  NULL                                           /* methods   */
};

/* vfl_object_float: address of the float_type structure. */
const object_type_t *vfl_object_float = &float_type;

