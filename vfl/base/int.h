
/* ensure once-only inclusion. */
#ifndef __VFL_INT_H__
#define __VFL_INT_H__

/* include c library headers. */
#include <stdlib.h>
#include <math.h>

/* include vfl headers. */
#include <vfl/base/object.h>

/* OBJECT_IS_INT(): check if an object is an integer.
 */
#define OBJECT_IS_INT(obj) \
  (OBJECT_TYPE(obj) == vfl_object_int)

/* int_t: structure for holding an integer.
 */
typedef struct {
  /* base structure members. */
  OBJECT_BASE;

  /* @val: integer value.
   */
  long val;
}
int_t;

/* function declarations (base/int.c): */

#define int_alloc() \
  (int_t*) obj_alloc(vfl_object_int)

int_t *int_alloc_with_value (const long val);

long int_get (const int_t *i);

void int_set (int_t *i, const long val);

/* available object types: */

extern const object_type_t *vfl_object_int;

#endif /* !__VFL_INT_H__ */

