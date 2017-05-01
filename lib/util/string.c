
/* include the string header. */
#include <vfl/util/string.h>

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
  str->val = malloc(1);
  if (!str->val)
    return 0;

  /* terminate the value and length. */
  str->val[0] = '\0';
  str->len = 0;

  /* return success. */
  return 1;
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
  str->val = realloc(str->val, strlen(val) + 1);
  if (!str->val)
    return 0;

  /* set the string length and value. */
  str->len = strlen(val);
  strcpy(str->val, val);

  /* return success. */
  return 1;
}

/* string_type: string type structure.
 */
static object_type_t string_type = {
  "string",                                      /* name      */
  sizeof(string_t),                              /* size      */

  (object_init_fn) string_init,                  /* init      */
  (object_copy_fn) string_copy,                  /* copy      */
  (object_free_fn) string_free,                  /* free      */

  NULL,                                          /* add       */
  NULL,                                          /* sub       */
  NULL,                                          /* mul       */
  NULL,                                          /* div       */

  NULL                                           /* methods   */
};

/* vfl_object_string: address of the string_type structure. */
const object_type_t *vfl_object_string = &string_type;

