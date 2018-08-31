
/* include the vfl list header. */
#include <vfl/vfl.h>

/* PyList_FromVector(): create a PyList object from a vector.
 */
PyObject*
PyList_FromVector (const Vector *x) {
  /* return an empty list if the input struct pointer is null
   * or if the vector has no elements.
   */
  if (!x || x->len == 0)
    return PyList_New(0);

  /* allocate a new list of the appropriate size. */
  PyObject *lst = PyList_New(x->len);
  if (!lst)
    return NULL;

  /* fill the list with new float objects. */
  for (Py_ssize_t i = 0; i < PyList_GET_SIZE(lst); i++)
    PyList_SET_ITEM(lst, i, PyFloat_FromDouble(vector_get(x, i)));

  /* return the created list. */
  return lst;
}

/* PySequence_AsVector(): create a vector from a PySequence object.
 */
Vector*
PySequence_AsVector (PyObject *pyseq) {
  /* fail if the input object is not a sequence. */
  if (!PySequence_Check(pyseq))
    return NULL;

  /* fail if the sequence has invalid size. */
  const Py_ssize_t n = PySequence_Length(pyseq);
  if (n < 1)
    return NULL;

  /* allocate new vector of the appropriate size. */
  Vector *x = vector_alloc(n);
  if (!x)
    return NULL;

  /* loop over the sequence items. */
  for (Py_ssize_t i = 0; i < n; i++) {
    /* get the current sequence item. */
    PyObject *obj = PySequence_GetItem(pyseq, i);
    if (!obj) {
      vector_free(x);
      return NULL;
    }

    /* try to pull a double from the item and lose our reference. */
    const double xi = PyFloat_AsDouble(obj);
    Py_DECREF(obj);

    /* check if the conversion to double failed. */
    if (PyErr_Occurred()) {
      vector_free(x);
      return NULL;
    }

    /* set the vector element. */
    vector_set(x, i, xi);
  }

  /* return the new vector. */
  return x;
}

