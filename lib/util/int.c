
/* include the integer header. */
#include <vfl/util/int.h>

/* int_init(): initialize an integer.
 *
 * arguments:
 *  @int: integer object structure pointer.
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
  int_t *i = (int_t*) obj_alloc(vfl_object_int);
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

/* int_add(): addition function for integers.
 */
int_t *int_add (const int_t *in1, const int_t *in2) {
  /* if both arguments are integers, return a result. */
  if (OBJECT_IS_INT(in1) && OBJECT_IS_INT(in2))
    return int_alloc_with_value(in1->val + in2->val);

  /* return no result. */
  return NULL;
}

/* int_sub(): subtraction function for integers.
 */
int_t *int_sub (const int_t *in1, const int_t *in2) {
  /* if both arguments are integers, return a result. */
  if (OBJECT_IS_INT(in1) && OBJECT_IS_INT(in2))
    return int_alloc_with_value(in1->val - in2->val);

  /* return no result. */
  return NULL;
}

/* int_mul(): multiplication function for integers.
 */
int_t *int_mul (const int_t *in1, const int_t *in2) {
  /* if both arguments are integers, return a result. */
  if (OBJECT_IS_INT(in1) && OBJECT_IS_INT(in2))
    return int_alloc_with_value(in1->val * in2->val);

  /* return no result. */
  return NULL;
}

/* int_div(): division function for integers.
 */
int_t *int_div (const int_t *in1, const int_t *in2) {
  /* if both arguments are integers, return a result. */
  if (OBJECT_IS_INT(in1) && OBJECT_IS_INT(in2))
    return int_alloc_with_value(in1->val / in2->val);

  /* return no result. */
  return NULL;
}

/* int_type: integer type structure.
 */
static object_type_t int_type = {
  "int",                                         /* name      */
  sizeof(int_t),                                 /* size      */

  (object_init_fn) int_init,                     /* init      */
  (object_copy_fn) int_copy,                     /* copy      */
  NULL,                                          /* free      */

  (object_binary_fn) int_add,                    /* add       */
  (object_binary_fn) int_sub,                    /* sub       */
  (object_binary_fn) int_mul,                    /* mul       */
  (object_binary_fn) int_div,                    /* div       */

  NULL                                           /* methods   */
};

/* vfl_object_int: address of the int_type structure. */
const object_type_t *vfl_object_int = &int_type;

