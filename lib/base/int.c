
/* include the required object headers. */
#include <vfl/base/int.h>
#include <vfl/base/float.h>
#include <vfl/base/string.h>

/* int_init(): initialize an integer.
 *
 * arguments:
 *  @i: integer object structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int int_init (int_t *i) {
  /* initialize the value and return success. */
  i->val = 0L;
  return 1;
}

/* int_copy(): copy the value of one integer to another.
 *
 * arguments:
 *  @i: source integer structure pointer.
 *  @idup: destination integer structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int int_copy (const int_t *i, int_t *idup) {
  /* copy the value and return success. */
  idup->val = i->val;
  return 1;
}

/* int_alloc_with_value(): allocate an integer object with
 * a specified value.
 *
 * arguments:
 *  @val: value to store in the new integer.
 *
 * returns:
 *  newly allocated and initialized integer object.
 */
int_t *int_alloc_with_value (const long val) {
  /* allocate a new integer. */
  int_t *i = int_alloc();
  if (!i)
    return NULL;

  /* set the integer value and return. */
  i->val = val;
  return i;
}

/* int_get(): get the value of an integer object.
 *
 * arguments:
 *  @i: integer structure pointer.
 *
 * returns:
 *  value of the integer object.
 */
long int_get (const int_t *i) {
  /* return the integer value. */
  return (i ? i->val : 0L);
}

/* int_set(): set the value of an integer object.
 *
 * arguments:
 *  @i: integer structure pointer.
 *  @val: new value to set.
 */
void int_set (int_t *i, const long val) {
  /* if possible, set the value. */
  if (i)
    i->val = val;
}

/* int_test(): assertion function for integers.
 *  - see object_test_fn() for details.
 */
int int_test (const int_t *obj) {
  /* return true for nonzero integers. */
  return (obj->val ? 1 : 0);
}

/* int_cmp(): comparison function for integers.
 *  - see object_comp_fn() for details.
 */
int int_cmp (const int_t *a, const int_t *b) {
  /* if both arguments are integers, return a result. */
  if (OBJECT_IS_INT(a) && OBJECT_IS_INT(b))
    return (a->val < b->val ? -1 : a->val > b->val ? 1 : 0);

  /* return no result. */
  return OBJECT_CMP_ERR;
}

/* int_add(): addition function for integers.
 *  - see object_binary_fn() for details.
 */
int_t *int_add (const int_t *a, const int_t *b) {
  /* if both arguments are integers, return a result. */
  if (OBJECT_IS_INT(a) && OBJECT_IS_INT(b))
    return int_alloc_with_value(a->val + b->val);

  /* return no result. */
  return NULL;
}

/* int_sub(): subtraction function for integers.
 *  - see object_binary_fn() for details.
 */
int_t *int_sub (const int_t *a, const int_t *b) {
  /* if both arguments are integers, return a result. */
  if (OBJECT_IS_INT(a) && OBJECT_IS_INT(b))
    return int_alloc_with_value(a->val - b->val);

  /* return no result. */
  return NULL;
}

/* int_mul(): multiplication function for integers.
 *  - see object_binary_fn() for details.
 */
int_t *int_mul (const int_t *a, const int_t *b) {
  /* if both arguments are integers, return a result. */
  if (OBJECT_IS_INT(a) && OBJECT_IS_INT(b))
    return int_alloc_with_value(a->val * b->val);

  /* return no result. */
  return NULL;
}

/* int_div(): division function for integers.
 *  - see object_binary_fn() for details.
 */
int_t *int_div (const int_t *a, const int_t *b) {
  /* if both arguments are integers, return a result. */
  if (OBJECT_IS_INT(a) && OBJECT_IS_INT(b))
    return int_alloc_with_value(a->val / b->val);

  /* return no result. */
  return NULL;
}

/* --- */

/* int_setprop_value(): set the value of an integer.
 *  - see object_setprop_fn() for details.
 */
static int int_setprop_value (int_t *i, object_t *val) {
  /* accept integers, floats, and strings. */
  if (OBJECT_IS_INT(val)) {
    /* set the value from the integer. */
    i->val = int_get((int_t*) val);
    return 1;
  }
  else if (OBJECT_IS_FLOAT(val)) {
    /* cast the float to an integer. */
    i->val = (long) float_get((flt_t*) val);
    return 1;
  }
  else if (OBJECT_IS_STRING(val)) {
    /* cast the string to an integer. */
    i->val = atol(string_get((string_t*) val));
    return 1;
  }

  /* invalid value type. */
  return 0;
}

/* int_properties: array of accessible object properties.
 */
static object_property_t int_properties[] = {
  { "value", NULL, (object_setprop_fn) int_setprop_value },
  { "val",   NULL, (object_setprop_fn) int_setprop_value },
  { "v",     NULL, (object_setprop_fn) int_setprop_value },
  { NULL, NULL, NULL }
};

/* --- */

/* int_type: integer type structure.
 */
static object_type_t int_type = {
  "int",                                         /* name      */
  sizeof(int_t),                                 /* size      */

  (object_init_fn) int_init,                     /* init      */
  (object_copy_fn) int_copy,                     /* copy      */
  NULL,                                          /* free      */
  (object_test_fn) int_test,                     /* test      */
  (object_comp_fn) int_cmp,                      /* cmp       */

  (object_binary_fn) int_add,                    /* add       */
  (object_binary_fn) int_sub,                    /* sub       */
  (object_binary_fn) int_mul,                    /* mul       */
  (object_binary_fn) int_div,                    /* div       */
  NULL,                                          /* pow       */

  NULL,                                          /* get       */
  NULL,                                          /* set       */
  int_properties,                                /* props     */
  NULL                                           /* methods   */
};

/* vfl_object_int: address of the int_type structure. */
const object_type_t *vfl_object_int = &int_type;

