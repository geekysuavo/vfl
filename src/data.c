
/* include the vfl header. */
#include <vfl/vfl.h>

/* define documentation strings: */

PyDoc_STRVAR(
  Data_doc,
"Data() -> Data object\n"
"\n");

PyDoc_STRVAR(
  Data_getset_dims_doc,
"Dimensionality of a dataset (read-only)\n"
"\n");

PyDoc_STRVAR(
  Data_method_augment_doc,
"Augment a dataset with new observations.\n"
"\n");

PyDoc_STRVAR(
  Data_method_write_doc,
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

  /* allocate a vector for the new datum. */
  Vector *xsrc = vector_alloc(self->D);
  if (!xsrc)
    return NULL;

  /* create a new datum to return. */
  Datum *dsrc = data_get(self, i);
  Datum *d = (Datum*) PyObject_CallObject((PyObject*) &Datum_Type, NULL);
  if (!d)
    return NULL;

  /* copy the datum properties. */
  d->p = dsrc->p;
  d->y = dsrc->y;
  d->x = xsrc;
  vector_copy(d->x, dsrc->x);

  /* return the new datum. */
  return (PyObject*) d;
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

  /* check that the value is a datum type. */
  if (!Datum_Check(v)) {
    PyErr_SetNone(PyExc_TypeError);
    return -1;
  }

  /* copy the datum information into the dataset. */
  if (!data_set(self, i, (Datum*) v)) {
    PyErr_SetNone(PyExc_RuntimeError);
    return -1;
  }

  /* return success. */
  return 0;
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
data_method_augment (Data *self, PyObject *args, PyObject *kwargs) {
  /* define the keyword argument list. */
  static char *kwlist[] = {
    "file", "datum", "data", "grid", "output", "outputs", NULL
  };

  /* parse the method arguments. */
  PyObject *fobj = NULL;
  PyObject *dobj = NULL;
  PyObject *Dobj = NULL;
  PyObject *pobj = NULL;
  PyObject *Pobj = NULL;
  Matrix *grid = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|O&$O!O!O&OO", kwlist,
                                   PyUnicode_FSConverter, &fobj,
                                   &Datum_Type, &dobj,
                                   &Data_Type, &Dobj,
                                   Matrix_Converter, &grid,
                                   &pobj, &Pobj))
                                     return NULL;

  /* if a grid was specified, do grid augmentation first, so we
   * can deallocate it and assume its null from here on out.
   */
  if (grid) {
    /* make sure 'output' and 'outputs' were not simultaneously given. */
    if (pobj && Pobj) {
      PyErr_SetString(PyExc_ValueError, "'output' and 'outputs' given");
      matrix_free(grid);
      return NULL;
    }

    /* check if multiple output indices were given. */
    if (Pobj) {
      /* check that the object is a sequence. */
      if (!PySequence_Check(Pobj)) {
        PyErr_SetString(PyExc_TypeError, "expected sequence 'outputs'");
        matrix_free(grid);
        return NULL;
      }

      /* loop over the outputs. */
      for (Py_ssize_t i = 0; i < PySequence_Length(Pobj); i++) {
        /* get the current sequence element. */
        pobj = PySequence_GetItem(Pobj, i);
        if (!pobj) {
          matrix_free(grid);
          return NULL;
        }

        /* get the current output value. */
        const size_t pval = PyLong_AsSize_t(pobj);
        Py_DECREF(pobj);

        /* check for proper conversion. */
        if (PyErr_Occurred()) {
          matrix_free(grid);
          return NULL;
        }

        /* augment from the grid. */
        if (!data_augment_from_grid(self, pval, grid)) {
          PyErr_SetString(PyExc_RuntimeError, "failed to augment from grid");
          matrix_free(grid);
          return NULL;
        }
      }
    }
    else {
      /* set the default output index value. */
      size_t pval = 0;

      /* check if an output index was specified. */
      if (pobj) {
        /* get the output index value. */
        pval = PyLong_AsSize_t(pobj);
        if (PyErr_Occurred()) {
          matrix_free(grid);
          return NULL;
        }
      }

      /* augment from the grid. */
      if (!data_augment_from_grid(self, pval, grid)) {
        PyErr_SetString(PyExc_RuntimeError, "failed to augment from grid");
        matrix_free(grid);
        return NULL;
      }
    }

    /* free the gridding matrix. */
    matrix_free(grid);
  }

  /* if a filename was given, read it into the dataset. */
  if (fobj && !data_fread(self, PyBytes_AsString(fobj))) {
    PyErr_SetNone(PyExc_IOError);
    return NULL;
  }

  /* if a datum was given, add it to the dataset. */
  if (dobj && !data_augment(self, (Datum*) dobj)) {
    PyErr_SetString(PyExc_RuntimeError, "failed to append datum");
    return NULL;
  }

  /* if a dataset was given, add it to the dataset. */
  if (Dobj && !data_augment_from_data(self, (Data*) Dobj)) {
    PyErr_SetString(PyExc_RuntimeError, "failed to append dataset");
    return NULL;
  }

  /* return nothing. */
  Py_RETURN_NONE;
}

/* data_method_write(): write the contents of a dataset to a file.
 */
static PyObject*
data_method_write (Data *self, PyObject *args, PyObject *kwargs) {
  /* define the keyword argument list. */
  static char *kwlist[] = { "file", NULL };

  /* parse the filename argument. */
  PyObject *fobj = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&", kwlist,
                                   PyUnicode_FSConverter, &fobj))
                                     return NULL;

  /* write the data to the file. */
  if (!data_fwrite(self, PyBytes_AsString(fobj))) {
    PyErr_SetNone(PyExc_IOError);
    return NULL;
  }

  /* return nothing. */
  Py_RETURN_NONE;
}

/* --- */

/* data_new(): allocation method for datasets.
 */
static PyObject*
data_new (PyTypeObject *type, PyObject *args, PyObject *kwargs) {
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
  /* creation and calling augment() behave exactly the same. */
  PyObject *obj = data_method_augment(self, args, kwargs);
  if (!obj)
    return -1;

  /* return success. */
  Py_DECREF(obj);
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
  { "dims",
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

