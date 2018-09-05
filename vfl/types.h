
/* ensure once-only inclusion. */
#ifndef __VFL_TYPES_H__
#define __VFL_TYPES_H__

/* VFL_TYPE_NEW(): macro function for declaring and defining
 * functions conforming to PyTypeObject->tp_new().
 */
#define VFL_TYPE_NEW(Typ) \
static PyObject* \
Typ ## _new (PyTypeObject *type, PyObject *args, PyObject *kwargs)

/* VFL_TYPE_INIT(): macro function for defining a PyTypeInit function
 * for subclasses of Model, Optim or Factor.
 */
#define VFL_TYPE_INIT(Typ) \
int Typ ## _Type_init (PyObject *mod) { \
  if (PyType_Ready(&Typ ## _Type) < 0) return -1; \
  Py_INCREF(&Typ ## _Type); \
  PyModule_AddObject(mod, #Typ, (PyObject*) &Typ ## _Type); \
  return 0; }

/* VFL_TYPE_DEF(): macro function for defining a PyTypeObject
 * structure for subclasses of Model, Optim or Factor.
 */
#define VFL_TYPE_DEF(Typ,Base,base) \
PyTypeObject Typ ## _Type = { \
  PyVarObject_HEAD_INIT(NULL, 0) \
  #base "." #Typ, \
  sizeof(Typ), \
  0, 0, 0, 0, 0, 0, 0, 0, \
  0, 0, 0, 0, 0, 0, 0, 0, \
  Py_TPFLAGS_DEFAULT, \
  Typ ## _doc, \
  0, 0, 0, 0, 0, 0, \
  Typ ## _methods, \
  0, \
  Typ ## _getset, \
  & Base ## _Type, \
  0, 0, 0, 0, 0, 0, \
  Typ ## _new };

/* VFL_TYPE(): macro function for defining a subtype of Model,
 * Optim or Factor.
 */
#define VFL_TYPE(Typ,Base,base) \
  VFL_TYPE_DEF(Typ, Base, base) \
  VFL_TYPE_INIT(Typ)

#endif /* !__VFL_TYPES_H__ */

