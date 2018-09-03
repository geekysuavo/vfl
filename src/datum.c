
/* include the vfl header. */
#include <vfl/vfl.h>

/* define documentation strings: */

PyDoc_STRVAR(
  Datum_doc,
"Datum() -> Datum object\n"
"\n");

PyDoc_STRVAR(
  Datum_getset_dims_doc,
"Dimensionality of a datum (read-only)\n"
"\n");

PyDoc_STRVAR(
  Datum_getset_output_doc,
"Output index of a datum (read/write)\n"
"\n");

PyDoc_STRVAR(
  Datum_getset_input_doc,
"Input location of a datum (read/write)\n"
"\n");

PyDoc_STRVAR(
  Datum_getset_value_doc,
"Observed value of a datum (read/write)"
"\n");

/* datum_seq_len(): method for getting datum dimensionalities.
 */
static Py_ssize_t
datum_seq_len (Datum *self) {
  /* return the current size of the input vector. */
  return (Py_ssize_t) (self->x ? self->x->len : 0);
}

/* datum_seq_get(): method for getting datum location entries.
 */
static PyObject*
datum_seq_get (Datum *self, Py_ssize_t i) {
  /* check that the index is in bounds. */
  const Py_ssize_t n = datum_seq_len(self);
  if (n == 0 || i < 0 || i >= n) {
    PyErr_SetNone(PyExc_IndexError);
    return NULL;
  }

  /* return the requested input value as a float. */
  return PyFloat_FromDouble(vector_get(self->x, i));
}

/* datum_seq_set(): method for setting datum location entries.
 */
static int
datum_seq_set (Datum *self, Py_ssize_t i, PyObject *v) {
  /* check that the index is in bounds. */
  const Py_ssize_t n = datum_seq_len(self);
  if (n == 0 || i < 0 || i >= n) {
    PyErr_SetNone(PyExc_IndexError);
    return -1;
  }

  /* get the new vector element value. */
  const double xi = PyFloat_AsDouble(v);
  if (PyErr_Occurred())
    return -1;

  /* set the value and return success. */
  vector_set(self->x, i, xi);
  return 0;
}

/* datum_get_dims(): method for getting datum dimensionalities.
 */
static PyObject*
datum_get_dims (Datum *self) {
  /* return the dimension count as an integer. */
  return PyLong_FromSize_t(self->x ? self->x->len : 0);
}

/* datum_get_output(): method for getting datum output indices.
 */
static PyObject*
datum_get_output (Datum *self) {
  /* return the output index as an integer. */
  return PyLong_FromSize_t(self->p);
}

/* datum_set_output(): method for setting datum output indices.
 */
static int
datum_set_output (Datum *self, PyObject *value, void *closure) {
  /* get the new value. */
  const size_t pval = PyLong_AsSize_t(value);
  if (PyErr_Occurred())
    return -1;

  /* set the output index return success. */
  self->p = pval;
  return 0;
}

/* datum_get_input(): method for getting datum input locations.
 */
static PyObject*
datum_get_input (Datum *self) {
  /* return the input location vector as a list. */
  return PyList_FromVector(self->x);
}

/* datum_set_input(): method for setting datum input locations.
 */
static int
datum_set_input (Datum *self, PyObject *value, void *closure) {
  /* get the new value. */
  Vector *xval = PySequence_AsVector(value);
  if (!xval)
    return -1;

  /* store the new vector. */
  vector_free(self->x);
  self->x = xval;

  /* return success. */
  return 0;
}

/* datum_get_value(): method for getting datum observed values.
 */
static PyObject*
datum_get_value (Datum *self) {
  /* return the observed value as a float. */
  return PyFloat_FromDouble(self->y);
}

/* datum_set_value(): method for setting datum observed values.
 */
static int
datum_set_value (Datum *self, PyObject *value, void *closure) {
  /* get the new value. */
  const double yval = PyFloat_AsDouble(value);
  if (PyErr_Occurred())
    return -1;

  /* set the observed value and return success. */
  self->y = yval;
  return 0;
}

/* --- */

/* datum_new(): allocation method for datum objects.
 */
static PyObject*
datum_new (PyTypeObject *type, PyObject *args, PyObject *kwargs) {
  /* allocate a new datum object. */
  Datum *self = (Datum*) type->tp_alloc(type, 0);
  if (!self)
    return NULL;

  /* initialize the output index and value. */
  self->y = 0.0;
  self->p = 0;

  /* initialize the location vector. */
  self->x = NULL;

  /* return the new object. */
  return (PyObject*) self;
}

/* datum_init(): initialization method for datum objects.
 */
static int
datum_init (Datum *self, PyObject *args, PyObject *kwargs) {
  /* define the keyword argument list. */
  static char *kwlist[] = { "output", "x", "y", NULL };

  /* parse the arguments. */
  PyObject *pobj = NULL;
  PyObject *xobj = NULL;
  PyObject *yobj = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|$OOO", kwlist,
                                   &pobj, &xobj, &yobj)) return -1;

  /* set the output index. */
  if (pobj && datum_set_output(self, pobj, NULL) < 0)
    return -1;

  /* set the input location. */
  if (xobj && datum_set_input(self, xobj, NULL) < 0)
    return -1;

  /* set the observed value. */
  if (yobj && datum_set_value(self, yobj, NULL) < 0)
    return -1;

  /* return success. */
  return 0;
}

/* datum_dealloc(): deallocation method for datum objects.
 */
static void
datum_dealloc (Datum *self) {
  /* free the location vector. */
  vector_free(self->x);

  /* release the object memory. */
  Py_TYPE(self)->tp_free((PyObject*) self);
}

/* datum_repr(): representation method for datum objects.
 */
static PyObject*
datum_repr (Datum *self) {
  /* build and return the representation string. */
  return PyUnicode_FromFormat("<vfl.Datum at 0x%x>", (long) self);
}

/* Datum_sequence: sequence definition structure for datum objects.
 */
static PySequenceMethods Datum_sequence = {
  (lenfunc) datum_seq_len,                       /* sq_length         */
  NULL,                                          /* sq_concat         */
  NULL,                                          /* sq_repeat         */
  (ssizeargfunc) datum_seq_get,                  /* sq_item           */
  NULL,
  (ssizeobjargproc) datum_seq_set,               /* sq_ass_item       */
  NULL,
  NULL,                                          /* sq_contains       */
  NULL,                                          /* sq_inplace_concat */
  NULL                                           /* sq_inplace_repeat */
};

/* Datum_getset: property definition structure for datum objects.
 */
static PyGetSetDef Datum_getset[] = {
  { "dims",
    (getter) datum_get_dims,
    NULL,
    Datum_getset_dims_doc,
    NULL
  },
  { "output",
    (getter) datum_get_output,
    (setter) datum_set_output,
    Datum_getset_output_doc,
    NULL
  },
  { "x",
    (getter) datum_get_input,
    (setter) datum_set_input,
    Datum_getset_input_doc,
    NULL
  },
  { "y",
    (getter) datum_get_value,
    (setter) datum_set_value,
    Datum_getset_value_doc,
    NULL
  },
  { NULL }
};

/* Datum_methods: method definition structure for datum objects.
 */
static PyMethodDef Datum_methods[] = {
  { NULL }
};

/* Datum_Type: type definition structure for datum objects.
 */
PyTypeObject Datum_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "vfl.Datum",                                   /* tp_name           */
  sizeof(Datum),                                 /* tp_basicsize      */
  0,                                             /* tp_itemsize       */
  (destructor) datum_dealloc,                    /* tp_dealloc        */
  0,                                             /* tp_print          */
  0,                                             /* tp_getattr        */
  0,                                             /* tp_setattr        */
  0,                                             /* tp_reserved       */
  (reprfunc) datum_repr,                         /* tp_repr           */
  0,                                             /* tp_as_number      */
  &Datum_sequence,                               /* tp_as_sequence    */
  0,                                             /* tp_as_mapping     */
  0,                                             /* tp_hash           */
  0,                                             /* tp_call           */
  (reprfunc) datum_repr,                         /* tp_str            */
  0,                                             /* tp_getattro       */
  0,                                             /* tp_setattro       */
  0,                                             /* tp_as_buffer      */
  Py_TPFLAGS_DEFAULT |
  Py_TPFLAGS_BASETYPE,                           /* tp_flags          */
  Datum_doc,                                     /* tp_doc            */
  0,                                             /* tp_traverse       */
  0,                                             /* tp_clear          */
  0,                                             /* tp_richcompare    */
  0,                                             /* tp_weaklistoffset */
  0,                                             /* tp_iter           */
  0,                                             /* tp_iternext       */
  Datum_methods,                                 /* tp_methods        */
  0,                                             /* tp_members        */
  Datum_getset,                                  /* tp_getset         */
  0,                                             /* tp_base           */
  0,                                             /* tp_dict           */
  0,                                             /* tp_descr_get      */
  0,                                             /* tp_descr_set      */
  0,                                             /* tp_dictoffset     */
  (initproc) datum_init,                         /* tp_init           */
  0,                                             /* tp_alloc          */
  datum_new                                      /* tp_new            */
};

/* Datum_Type_init(): type initialization function for datum objects.
 */
int
Datum_Type_init (PyObject *mod) {
  /* finalize the type object. */
  if (PyType_Ready(&Datum_Type) < 0)
    return -1;

  /* take a reference to the type and add it to the module. */
  Py_INCREF(&Datum_Type);
  PyModule_AddObject(mod, "Datum", (PyObject*) &Datum_Type);

  /* return success. */
  return 0;
}

