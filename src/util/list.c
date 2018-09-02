
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

/* PyList_FromMatrix(): create a PyList object from a matrix.
 */
PyObject*
PyList_FromMatrix (const Matrix *A) {
  /* return an empty list if the input struct pointer is null
   * or if the matrix has no elements.
   */
  if (!A || A->rows == 0 || A->cols == 0)
    return PyList_New(0);

  /* allocate a new list to store the rows. */
  PyObject *lst = PyList_New(A->rows);
  if (!lst)
    return NULL;

  /* fill the list with new list objects. */
  for (Py_ssize_t i = 0; i < PyList_GET_SIZE(lst); i++) {
    /* allocate a list from the current row. */
    VectorView ai = matrix_row(A, i);
    PyObject *row = PyList_FromVector(&ai);
    if (!row) {
      Py_DECREF(lst);
      return NULL;
    }

    /* store the created list into the rows list. */
    PyList_SET_ITEM(lst, i, row);
  }

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

  /* allocate a new vector of the appropriate size. */
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

/* PySequence_AsMatrix(): create a matrix from a PySequence object.
 */
Matrix*
PySequence_AsMatrix (PyObject *pyseq) {
  /* fail if the input object is not a sequence. */
  if (!PySequence_Check(pyseq))
    return NULL;

  /* fail if the sequence has invalid size. */
  const Py_ssize_t nr = PySequence_Length(pyseq);
  if (nr < 1)
    return NULL;

  /* declare the matrix to be allocated. */
  Matrix *A = NULL;

  /* loop over the sequence of rows. */
  for (Py_ssize_t i = 0; i < nr; i++) {
    /* get the current row. */
    PyObject *row = PySequence_GetItem(pyseq, i);
    if (!row) {
      matrix_free(A);
      return NULL;
    }

    /* build a temporary vector from the current row. */
    Vector *x = PySequence_AsVector(row);
    Py_DECREF(row);
    if (!x) {
      matrix_free(A);
      return NULL;
    }

    /* on the first iteration... */
    if (i == 0) {
      /* ... allocate the output matrix. */
      A = matrix_alloc(nr, x->len);
      if (!A)
        return NULL;
    }
    else if (x->len != A->cols) {
      /* otherwise fail if the current vector length does not
       * match the matrix column count.
       */
      matrix_free(A);
      vector_free(x);
      return NULL;
    }

    /* copy the vector into the matrix. */
    VectorView ai = matrix_row(A, i);
    vector_copy(&ai, x);
    vector_free(x);
  }

  /* return the new matrix. */
  return A;
}

/* Vector_Converter(): converter function for parsing to Vector.
 *  - see PyArg_ParseTupleAndKeywords() for details.
 */
int
Vector_Converter (PyObject *obj, void *addr) {
  /* attempt to create a vector from the object. */
  Vector *x = PySequence_AsVector(obj);
  if (!x) {
    PyErr_SetString(PyExc_TypeError, "conversion to vector failed");
    return 0;
  }

  /* set the address and return success. */
  *(Vector**) addr = x;
  return 1;
}

/* Matrix_Converter(): converter function for parsing to Matrix.
 *  - see PyArg_ParseTupleAndKeywords() for details.
 */
int Matrix_Converter (PyObject *obj, void *addr) {
  /* attempt to create a matrix from the object. */
  Matrix *A = PySequence_AsMatrix(obj);
  if (!A) {
    PyErr_SetString(PyExc_TypeError, "conversion to matrix failed");
    return 0;
  }

  /* set the address and return success. */
  *(Matrix**) addr = A;
  return 1;
}

