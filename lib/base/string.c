
/* include the string header. */
#include <vfl/base/string.h>

/* include the required object headers. */
#include <vfl/base/map.h>
#include <vfl/base/list.h>
#include <vfl/base/float.h>

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
  char *val = realloc(str->val, sz);
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
  string_t *str = string_alloc();
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
 *
 * returns:
 *  integer indicating success (1) or failure (0).
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

  /* copy the new value and return success. */
  strcpy(str->val, val);
  return 1;
}

/* string_append(): append character data to a string object.
 *
 * arguments:
 *  @str: string structure pointer.
 *  @val: new value to append.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int string_append (string_t *str, const char *val) {
  /* check the string pointer. */
  if (!str)
    return 0;

  /* check for null values. */
  if (!val)
    return 1;

  /* reallocate the string buffer. */
  if (!string_set_length(str, str->len + strlen(val)))
    return 0;

  /* concatenate the new value and return success. */
  strcat(str->val, val);
  return 1;
}

/* --- */

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

/* --- */

/* string_getprop_len(): length property getter.
 *  - see object_getprop_fn() for details.
 */
static int_t *string_getprop_len (const string_t *str) {
  /* return the string length as a new integer. */
  return int_alloc_with_value(str->len);
}

/* string_properties: array of accessible object properties.
 */
static object_property_t string_properties[] = {
  { "len", (object_getprop_fn) string_getprop_len, NULL },
  { NULL, NULL, NULL }
};

/* --- */

/* string_method_format(): build a formatted string from a template
 * and a set of corresponding arguments.
 *  - see object_method_fn() for details.
 */
static string_t *string_method_format (string_t *fmtstr, map_t *args) {
  /* get the values argument. */
  object_t *vals = map_get(args, "values");
  if (!vals) vals = map_get(args, "vals");
  if (!vals) vals = map_get(args, "v");
  if (!vals)
    return NULL;

  /* admit only list arguments. */
  if (!OBJECT_IS_LIST(vals))
    return NULL;

  /* allocate a new string for output. */
  string_t *str = string_alloc();
  if (!str)
    return NULL;

  /* initialize variables for format string scanning. */
  char *pa = fmtstr->val;
  char *pb = pa;
  size_t i = 0;

  /* loop over the format string. */
  do {
    /* find the next format specifier. */
    pb = strchr(pa, '%');

    /* check if no specifier was found. */
    if (!pb) {
      string_append(str, pa);
      break;
    }

    /* append the substring preceding the format specifier. */
    *pb = '\0';
    string_append(str, pa);
    *pb = '%';
    pb++;

    /* break on early termination. */
    if (*pb == '\0')
      break;

    /* handle escaped percent characters. */
    if (*pb == '%') {
      string_append(str, "%");
      pa = pb + 1;
      continue;
    }

    /* loop until a type specifier is located. */
    pa = pb - 1;
    while (*pb && !(*pb == 'd' || *pb == 'u' ||
                    *pb == 'f' || *pb == 'e' ||
                    *pb == 'E' || *pb == 'g' ||
                    *pb == 'G' || *pb == 's')) pb++;

    /* get the current object from the values list. */
    object_t *val = list_get((list_t*) vals, i);
    i++;

    /* temporarily terminate the format specifier. */
    char tmp = pb[1];
    pb[1] = '\0';

    /* append based on the format specifier. */
    char buf[1024];
    if (*pb == 'd' || *pb == 'u') {
      if (!OBJECT_IS_INT(val)) goto fail;
      snprintf(buf, 1024, pa, int_get((int_t*) val));
      if (!string_append(str, buf))
        goto fail;
    }
    else if (*pb == 'f' || *pb == 'e' || *pb == 'E' ||
             *pb == 'g' || *pb == 'G') {
      if (!OBJECT_IS_NUM(val)) goto fail;
      snprintf(buf, 1024, pa, num_get(val));
      if (!string_append(str, buf))
        goto fail;
    }
    else if (*pb == 's') {
      if (!OBJECT_IS_STRING(val) ||
          !string_append(str, string_get((string_t*) val)))
        goto fail;
    }

    /* reverse the temporary termination and pass the format specifier. */
    pb[1] = tmp;
    pa = pb + 1;
  }
  while (1);

  /* return the newly allocated string. */
  return str;

fail:
  /* free the output string and return failure. */
  obj_release((object_t*) str);
  return NULL;
}

/* string_methods: array of callable object methods.
 */
static object_method_t string_methods[] = {
  { "format", (object_method_fn) string_method_format },
  { NULL, NULL }
};

/* --- */

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
  NULL,                                          /* pow       */

  NULL,                                          /* get       */
  NULL,                                          /* set       */
  string_properties,                             /* props     */
  string_methods                                 /* methods   */
};

/* vfl_object_string: address of the string_type structure. */
const object_type_t *vfl_object_string = &string_type;

