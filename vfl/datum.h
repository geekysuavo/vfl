
/* ensure once-only inclusion. */
#ifndef __VFL_DATUM_H__
#define __VFL_DATUM_H__

/* include vfl headers. */
#include <vfl/util/vector.h>

/* OBJECT_IS_DATUM(): check if an object is a datum.
 */
#define OBJECT_IS_DATUM(obj) \
  (OBJECT_TYPE(obj) == vfl_object_datum)

/* datum_t: structure for holding a single observation.
 */
typedef struct {
  /* base structure members. */
  OBJECT_BASE;

  /* properties of each observation:
   *  @p: observation output index.
   *  @x: observation location.
   *  @y: observed value.
   */
  unsigned int p;
  vector_t *x;
  double y;
}
datum_t;

/* function declarations (datum.c): */

int datum_cmp (const datum_t *d1, const datum_t *d2);

/* available object types: */

extern const object_type_t *vfl_object_datum;

#endif /* !__VFL_DATUM_H__ */

