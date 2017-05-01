
/* include the string and integer headers. */
#include <vfl/base/string.h>
#include <vfl/base/int.h>

/* string_set_length(): set the amount of available memory
 * in a string, but do not manage the contents.
 *
 * arguments:
 *  @str: string structure pointer.
 *  @len: new length to set.
 *
 * returns:
 *  integer indicating resize success (1) or failure (0).
 */
static int string_set_length (string_t *str, const size_t len) {
  /* check the input pointer. */
  if (!str)
    return 0;

  /* reallocate the string value. */
  const size_t sz = len + 1;
  char *val = realloc (str->val, sz);
  if (!val)
    return 0;

  /* store the new length and value. */
  str->len = len;
  str->val = val;

  /* null-terminate the value. */
  str->val[len] = '\0';

  /* return success. */
  return 1;
}

/* string_init(): initialize a string.
 *
 * arguments:
 *  @str: string object structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int string_init (string_t *str) {
  /* initialize the value. */
  str->val = NULL;
  return string_set_length(str, 0);
}

/* string_copy(): copy the value of one string to another.
 *
 * arguments:
 *  @s: source string structure pointer.
 *  @sdup: destination string structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int string_copy (const string_t *s, string_t *sdup) {
  /* copy the value and return the result. */
  return string_set(sdup, string_get(s));
}

/* string_free(): free the contents of a string.
 *
 * arguments:
 *  @str: string structure pointer to free.
 */
void string_free (string_t *str) {
  /* free the string value. */
  free(str->val);
}

/* string_alloc_with_value(): allocate a string object with
 * a specified value.
 *
 * arguments:
 *  @val: value to store in the new string.
 *
 * returns:
 *  newly allocated and initialized string object.
 */
string_t *string_alloc_with_value (const char *val) {
  /* allocate a new string. */
  string_t *str = (string_t*) obj_alloc(vfl_object_string);
  if (!str)
    return NULL;

  /* set the string value. */
  if (!string_set(str, val)) {
    obj_free((object_t*) str);
    return NULL;
  }

  /* return the new string. */
  return str;
}

/* string_get(): get the value of a string object.
 *
 * arguments:
 *  @str: string structure pointer.
 *
 * returns:
 *  value of the string object.
 */
char *string_get (const string_t *str) {
  /* return the string value. */
  return (str ? str->val : NULL);
}

/* string_set(): set the value of a string object.
 *
 * arguments:
 *  @str: string structure pointer.
 *  @val: new value to set.
 */
int string_set (string_t *str, const char *val) {
  /* check the string pointer. */
  if (!str)
    return 0;

  /* check for null values. */
  if (!val)
    return string_set(str, "");

  /* reallocate the string buffer. */
  if (!string_set_length(str, strlen(val)))
    return 0;

  /* copy the new value. */
  strcpy(str->val, val);

  /* return success. */
  return 1;
}

/* string_add(): addition function for strings.
 */
string_t *string_add (const object_t *a, const object_t *b) {
  /* check the input arguments. */
  if (OBJECT_IS_STRING(a) && OBJECT_IS_STRING(b)) {
    /* cast the arguments to strings. */
    const string_t *sa = (string_t*) a;
    const string_t *sb = (string_t*) b;

    /* get the new string length. */
    const size_t len = sa->len + sb->len;

    /* allocate a new string. */
    string_t *str = string_alloc();
    if (str && string_set_length(str, len)) {
      /* store the new string value. */
      strcpy(str->val, sa->val);
      strcat(str->val, sb->val);
      return str;
    }
  }

  /* return no result. */
  return NULL;
}

/* string_mul(): multiplication function for strings.
 */
string_t *string_mul (const object_t *a, const object_t *b) {
  /* declare required variables:
   *  @sobj: string object in the product.
   *  @iobj: integer multiplier.
   */
  string_t *sobj = NULL;
  int_t *iobj = NULL;

  /* check the argument types. */
  if (OBJECT_IS_STRING(a) && OBJECT_IS_INT(b)) {
    sobj = (string_t*) a;
    iobj = (int_t*) b;
  }
  else if (OBJECT_IS_INT(a) && OBJECT_IS_STRING(b)) {
    sobj = (string_t*) b;
    iobj = (int_t*) a;
  }
  else
    return NULL;

  /* get the integer value. */
  const long ival = int_get(iobj);
  if (ival <= 0)
    return NULL;

  /* determine the new lengths. */
  const size_t len = sobj->len;
  const size_t newlen = len * ival;

  /* allocate a new string. */
  string_t *str = string_alloc();
  if (!str || !string_set_length(str, newlen))
    return NULL;

  /* store the new string value. */
  strcpy(str->val, "");
  for (size_t i = 0; i < (size_t) ival; i++)
    strcat(str->val, sobj->val);

  /* return the new string. */
  return str;
}

/* string_type: string type structure.
 */
static object_type_t string_type = {
  "string",                                      /* name      */
  sizeof(string_t),                              /* size      */

  (object_init_fn) string_init,                  /* init      */
  (object_copy_fn) string_copy,                  /* copy      */
  (object_free_fn) string_free,                  /* free      */

  (object_binary_fn) string_add,                 /* add       */
  NULL,                                          /* sub       */
  (object_binary_fn) string_mul,                 /* mul       */
  NULL,                                          /* div       */

  NULL,                                          /* get       */
  NULL,                                          /* set       */
  NULL,                                          /* props     */
  NULL                                           /* methods   */
};

/* vfl_object_string: address of the string_type structure. */
const object_type_t *vfl_object_string = &string_type;

