
/* ensure once-only inclusion. */
#ifndef __VFL_FLOAT_H__
#define __VFL_FLOAT_H__

/* include c library headers. */
#include <stdlib.h>
#include <math.h>

/* include vfl headers. */
#include <vfl/base/int.h>

/* OBJECT_IS_FLOAT(): check if an object is a float.
 */
#define OBJECT_IS_FLOAT(obj) \
  (OBJECT_TYPE(obj) == vfl_object_float)

/* OBJECT_IS_NUM(): check if an object is a scalar number.
 */
#define OBJECT_IS_NUM(obj) \
  (OBJECT_IS_INT(obj) || OBJECT_IS_FLOAT(obj))

/* flt_t: structure for holding a float.
 */
typedef struct {
  /* @type: basic object type information.
   * @val: float value.
   */
  object_type_t *type;
  double val;
}
flt_t;

/* function declarations (base/float.c): */

#define float_alloc() \
  (flt_t*) obj_alloc(vfl_object_float)

flt_t *float_alloc_with_value (const double val);

double num_get (const object_t *num);

double float_get (const flt_t *f);

void float_set (flt_t *f, const double val);

/* available object types: */

extern const object_type_t *vfl_object_float;

#endif /* !__VFL_FLOAT_H__ */

