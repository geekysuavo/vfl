
/* ensure once-only inclusion. */
#ifndef __VFL_STRING_H__
#define __VFL_STRING_H__

/* include c library headers. */
#include <stdlib.h>
#include <string.h>

/* include vfl headers. */
#include <vfl/base/object.h>

/* OBJECT_IS_STRING(): check if an object is a string.
 */
#define OBJECT_IS_STRING(obj) \
  (OBJECT_TYPE(obj) == vfl_object_string)

/* string_t: structure for holding a string.
 */
typedef struct {
  /* base structure members. */
  OBJECT_BASE;

  /* @len: string length.
   * @val: string value.
   */
  size_t len;
  char *val;
}
string_t;

/* function declarations (base/string.c): */

#define string_alloc() \
  (string_t*) obj_alloc(vfl_object_string)

string_t *string_alloc_with_value (const char *val);

char *string_get (const string_t *str);

int string_set (string_t *str, const char *val);

int string_append (string_t *str, const char *val);

int string_append_list (string_t *str, const string_t *fmt,
                        object_t *vals);

/* available object types: */

extern const object_type_t *vfl_object_string;

#endif /* !__VFL_STRING_H__ */

