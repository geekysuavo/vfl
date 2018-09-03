
/* include the vfl list header. */
#include <vfl/vfl.h>

/* PySize_t_Converter(): converter function for parsing to size_t.
 *  - see PyArg_ParseTupleAndKeywords() for details.
 */
int PySize_t_Converter (PyObject *obj, void *addr) {
  /* attempt to read a size_t from the object. */
  const size_t val = PyLong_AsSize_t(obj);
  if (PyErr_Occurred())
    return 0;

  /* set the address and return success. */
  *(size_t*) addr = val;
  return 1;
}

