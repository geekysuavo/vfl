
/* ensure once-only inclusion. */
#ifndef __VFL_MATH_H__
#define __VFL_MATH_H__

/* include vfl headers. */
#include <vfl/base/object.h>

/* OBJECT_IS_MATH(): check if an object is a mathlib.
 */
#define OBJECT_IS_MATH(obj) \
  (OBJECT_TYPE(obj) == vfl_object_math)

/* function declarations (base/math.c): */

#define math_alloc() \
  obj_alloc(vfl_object_math)

/* available object types: */

extern const object_type_t *vfl_object_math;

#endif /* !__VFL_MATH_H__ */

