
/* ensure once-only inclusion. */
#ifndef __VFL_DATUM_H__
#define __VFL_DATUM_H__

/* include vfl headers. */
#include <vfl/util/vector.h>

/* Datum_Check(): macro to check if a PyObject is a Datum.
 */
#define Datum_Check(v) (Py_TYPE(v) == &Datum_Type)

/* Datum_Type: globally available datum type structure.
 */
PyAPI_DATA(PyTypeObject) Datum_Type;

/* Datum: structure for holding a single observation.
 */
typedef struct {
  /* object base. */
  PyObject_HEAD

  /* properties of each observation:
   *  @p: observation output index.
   *  @x: observation location.
   *  @y: observed value.
   */
  size_t p;
  Vector *x;
  double y;
}
Datum;

/* function declarations (datum-cmp.c): */

int datum_cmp (const Datum *d1, const Datum *d2);

#endif /* !__VFL_DATUM_H__ */

