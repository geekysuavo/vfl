
/* include the vfl header. */
#include <vfl/vfl.h>

/* define documentation strings: */

PyDoc_STRVAR(
  Data_doc,
"Data() -> Data object\n"
"\n");

PyDoc_STRVAR(
  Data_getset_len_doc,
"Number of observations in a dataset (read-only)\n"
"\n");

PyDoc_STRVAR(
  Data_getset_dims_doc,
"Dimensionality of a dataset (read-only)\n"
"\n");

PyDoc_STRVAR(
  Data_method_augment_doc,
"augment(data=None, datum=None)\n"
"augment(grid, output=0, outputs=None)\n"
"Augment a dataset with new observations.\n"
"\n");

PyDoc_STRVAR(
  Data_method_write_doc,
"write(file)\n"
"Write the contents of a dataset to a file.\n"
"\n");

/* data_seq_len(): method for getting dataset sizes.
 */
static Py_ssize_t
data_seq_len (Data *self) {
  /* return the current size of the input vector. */
  return (Py_ssize_t) self->N;
}

/* data_seq_get(): method for getting dataset entries.
 */
static PyObject*
data_seq_get (Data *self, Py_ssize_t i) {
  /* check that the index is in bounds. */
  const Py_ssize_t n = data_seq_len(self);
  if (n == 0 || i < 0 || i >= n) {
    PyErr_SetNone(PyExc_IndexError);
    return NULL;
  }

  /* FIXME: implement data_seq_get() */ return NULL;
}

/* data_seq_set(): method for setting dataset entries.
 */
static int
data_seq_set (Data *self, Py_ssize_t i, PyObject *v) {
  /* check that the index is in bounds. */
  const Py_ssize_t n = data_seq_len(self);
  if (n == 0 || i < 0 || i >= n) {
    PyErr_SetNone(PyExc_IndexError);
    return -1;
  }

  /* FIXME: implement data_seq_set() */

  /* return success. */
  return 0;
}

/* data_get_len(): method for getting dataset observation counts.
 */
static PyObject*
data_get_len (Data *self) {
  return PyLong_FromSize_t(self->N);
}

/* data_get_dims(): method for getting dataset dimensionalities.
 */
static PyObject*
data_get_dims (Data *self) {
  return PyLong_FromSize_t(self->D);
}

/* --- */

/* data_method_augment(): augment a dataset with new points.
 */
static PyObject*
data_method_augment (Data *self, PyObject *args, PyObject *keywords) {
  /* FIXME: implement data_method_augment() */

  /* return nothing. */
  Py_RETURN_NONE;
}

/* data_method_write(): write the contents of a dataset to a file.
 */
static PyObject*
data_method_write (Data *self, PyObject *args, PyObject *keywords) {
  /* FIXME: implement data_method_write() */

  /* return nothing. */
  Py_RETURN_NONE;
}

/* --- */

/* data_new(): allocation method for datasets.
 */
static PyObject*
data_new (PyTypeObject *type, PyObject *args, PyObject *keywords) {
  /* allocate a new dataset. */
  Data *self = (Data*) type->tp_alloc(type, 0);
  if (!self)
    return NULL;

  /* initialize the size of the dataset. */
  self->N = 0;
  self->D = 0;

  /* initialize the data array. */
  self->data = NULL;

  /* initialize the swap datum. */
  self->swp.p = 0;
  self->swp.x = NULL;
  self->swp.y = 0.0;

  /* return the new object. */
  return (PyObject*) self;
}

/* data_init(): initialization method for datasets.
 */
static int
data_init (Data *self, PyObject *args, PyObject *kwargs) {
  /* define the list of accepted keywords. */
  const char *kwlist[] = { "file", "grid", NULL };

  /* FIXME: implement data_init() */

  /* return success. */
  return 0;
}

/* data_dealloc(): deallocation method for datasets.
 */
static void
data_dealloc (Data *self) {
  /* free the array of observations. */
  free(self->data);

  /* free the swap vector. */
  vector_free(self->swp.x);

  /* release the object memory. */
  Py_TYPE(self)->tp_free((PyObject*) self);
}

/* data_repr(): representation method for datasets.
 */
static PyObject*
data_repr (Data *self) {
  /* build and return the representation string. */
  return PyUnicode_FromFormat("<vfl.Data at 0x%lx>", (long) self);
}

/* Data_sequence: sequence definition structure for datasets.
 */
static PySequenceMethods Data_sequence = {
  (lenfunc) data_seq_len,                        /* sq_length         */
  NULL,                                          /* sq_concat         */
  NULL,                                          /* sq_repeat         */
  (ssizeargfunc) data_seq_get,                   /* sq_item           */
  NULL,
  (ssizeobjargproc) data_seq_set,                /* sq_ass_item       */
  NULL,
  NULL,                                          /* sq_contains       */
  NULL,                                          /* sq_inplace_concat */
  NULL                                           /* sq_inplace_repeat */
};

/* Data_getset: property definition structure for datasets.
 */
static PyGetSetDef Data_getset[] = {
  { "N",
    (getter) data_get_len,
    NULL,
    Data_getset_len_doc,
    NULL
  },
  { "D",
    (getter) data_get_dims,
    NULL,
    Data_getset_dims_doc,
    NULL
  },
  { NULL }
};

/* Data_methods: method definition structure for datasets.
 */
static PyMethodDef Data_methods[] = {
  { "augment",
    (PyCFunction) data_method_augment,
    METH_VARARGS | METH_KEYWORDS,
    Data_method_augment_doc
  },
  { "write",
    (PyCFunction) data_method_write,
    METH_VARARGS | METH_KEYWORDS,
    Data_method_write_doc
  },
  { NULL }
};

/* Data_Type: type definition structure for datasets.
 */
PyTypeObject Data_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "vfl.Data",                                    /* tp_name           */
  sizeof(Data),                                  /* tp_basicsize      */
  0,                                             /* tp_itemsize       */
  (destructor) data_dealloc,                     /* tp_dealloc        */
  0,                                             /* tp_print          */
  0,                                             /* tp_getattr        */
  0,                                             /* tp_setattr        */
  0,                                             /* tp_reserved       */
  (reprfunc) data_repr,                          /* tp_repr           */
  0,                                             /* tp_as_number      */
  &Data_sequence,                                /* tp_as_sequence    */
  0,                                             /* tp_as_mapping     */
  0,                                             /* tp_hash           */
  0,                                             /* tp_call           */
  (reprfunc) data_repr,                          /* tp_str            */
  0,                                             /* tp_getattro       */
  0,                                             /* tp_setattro       */
  0,                                             /* tp_as_buffer      */
  Py_TPFLAGS_DEFAULT |
  Py_TPFLAGS_BASETYPE,                           /* tp_flags          */
  Data_doc,                                      /* tp_doc            */
  0,                                             /* tp_traverse       */
  0,                                             /* tp_clear          */
  0,                                             /* tp_richcompare    */
  0,                                             /* tp_weaklistoffset */
  0,                                             /* tp_iter           */
  0,                                             /* tp_iternext       */
  Data_methods,                                  /* tp_methods        */
  0,                                             /* tp_members        */
  Data_getset,                                   /* tp_getset         */
  0,                                             /* tp_base           */
  0,                                             /* tp_dict           */
  0,                                             /* tp_descr_get      */
  0,                                             /* tp_descr_set      */
  0,                                             /* tp_dictoffset     */
  (initproc) data_init,                          /* tp_init           */
  0,                                             /* tp_alloc          */
  data_new                                       /* tp_new            */
};

/* Data_Type_init(): type initialization function for datasets.
 */
int
Data_Type_init (PyObject *mod) {
  /* finalize the type object. */
  if (PyType_Ready(&Data_Type) < 0)
    return -1;

  /* take a reference to the type and add it to the module. */
  Py_INCREF(&Data_Type);
  PyModule_AddObject(mod, "Data", (PyObject*) &Data_Type);

  /* return success. */
  return 0;
}

