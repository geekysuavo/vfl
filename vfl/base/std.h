
/* ensure once-only inclusion. */
#ifndef __VFL_STD_H__
#define __VFL_STD_H__

/* include c library headers. */
#include <stdlib.h>
#include <string.h>

/* include vfl headers. */
#include <vfl/base/object.h>

/* OBJECT_IS_STD(): check if an object is a stdlib.
 */
#define OBJECT_IS_STD(obj) \
  (OBJECT_TYPE(obj) == vfl_object_std)

/* function declarations (base/std.c): */

#define std_alloc() \
  obj_alloc(vfl_object_std)

/* available object types: */

extern const object_type_t *vfl_object_std;

#endif /* !__VFL_STD_H__ */

