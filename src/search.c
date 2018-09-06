
/* include the vfl header. */
#include <vfl/vfl.h>

/* define documentation strings: */

PyDoc_STRVAR(
  Search_doc,
"Search() -> Search object\n"
"\n");

PyDoc_STRVAR(
  Search_getset_model_doc,
"Model used to compute posterior covariance (read/write)\n"
"\n");

PyDoc_STRVAR(
  Search_getset_data_doc,
"Dataset used to train the predictive model (read/write)\n"
"\n");

PyDoc_STRVAR(
  Search_getset_grid_doc,
"Grid of points to search at each execution (read/write)\n"
"\n");

PyDoc_STRVAR(
  Search_getset_outputs_doc,
"Function output(s) to search at each execution (read/write)\n"
"\n");

PyDoc_STRVAR(
  Search_method_execute_doc,
"Find the next location with maximum posterior variance.\n"
"\n");

/* declare private search functions: */

void free_buffers (Search *S);
void free_kernel (Search *S);

/* --- */

/* Search_get_model(): method for getting search models.
 */
static PyObject*
Search_get_model (Search *self) {
  /* return nothing if the model is null. */
  if (!self->mdl)
    Py_RETURN_NONE;

  /* return a new reference to the model. */
  Py_INCREF(self->mdl);
  return (PyObject*) self->mdl;
}

/* Search_set_model(): method for setting search models.
 */
static int
Search_set_model (Search *self, PyObject *value, void *closure) {
  /* check that the value is a model. */
  if (!value || !Model_Check(value)) {
    PyErr_SetNone(PyExc_TypeError);
    return -1;
  }

  /* attempt to set the model. */
  if (!search_set_model(self, (Model*) value)) {
    PyErr_SetString(PyExc_RuntimeError, "failed to set model");
    return -1;
  }

  /* return success. */
  return 0;
}

/* Search_get_data(): method for getting search datasets.
 */
static PyObject*
Search_get_data (Search *self) {
  /* return nothing if the dataset is null. */
  if (!self->dat)
    Py_RETURN_NONE;

  /* return a new reference to the dataset. */
  Py_INCREF(self->dat);
  return (PyObject*) self->dat;
}

/* Search_set_data(): method for setting search datasets.
 */
static int
Search_set_data (Search *self, PyObject *value, void *closure) {
  /* check that the value is a dataset. */
  if (!value || !Data_Check(value)) {
    PyErr_SetNone(PyExc_TypeError);
    return -1;
  }

  /* attempt to set the dataset. */
  if (!search_set_data(self, (Data*) value)) {
    PyErr_SetString(PyExc_RuntimeError, "failed to set dataset");
    return -1;
  }

  /* return success. */
  return 0;
}

/* Search_get_grid(): method for getting search grids.
 */
static PyObject*
Search_get_grid (Search *self) {
  /* cast the grid matrix to a list. */
  PyObject *lst = PyList_FromMatrix(self->grid);
  if (!lst) {
    PyErr_SetNone(PyExc_RuntimeError);
    return NULL;
  }

  /* return the new list. */
  return lst;
}

/* Search_set_grid(): method for setting search grids.
 */
static int
Search_set_grid (Search *self, PyObject *value, void *closure) {
  /* get the new value. */
  Matrix *gval = PySequence_AsMatrix(value);
  if (!gval)
    return -1;

  /* attempt to set the new grid. */
  if (!search_set_grid(self, gval)) {
    PyErr_SetString(PyExc_ValueError, "matrix is not a valid grid");
    matrix_free(gval);
    return -1;
  }

  /* return success. */
  return 0;
}

/* Search_get_outputs(): method for getting search output counts.
 */
static PyObject*
Search_get_outputs (Search *self) {
  /* return the output count as an integer. */
  return PyLong_FromSize_t(self->K);
}

/* Search_set_outputs(): method for setting search output counts.
 */
static int
Search_set_outputs (Search *self, PyObject *value, void *closure) {
  /* get the new value. */
  const size_t v = PyLong_AsSize_t(value);
  if (PyErr_Occurred())
    return -1;

  /* check that the count is nonzero. */
  if (v == 0) {
    PyErr_SetNone(PyExc_ValueError);
    return -1;
  }

  /* set the output count. */
  if (!search_set_outputs(self, v)) {
    PyErr_SetNone(PyExc_RuntimeError);
    return -1;
  }

  /* return success. */
  return 0;
}

/* --- */

static PyObject*
Search_method_execute (Search *self, PyObject *args) {
  /* get the current search dimensionality. */
  const size_t D = (self->mdl ? self->mdl->D : 0);
  if (!D) {
    PyErr_SetString(PyExc_RuntimeError, "cannot search without a model");
    return NULL;
  }

  /* allocate a temporary vector for the result. */
  Vector *x = vector_alloc(D);
  if (!x) {
    PyErr_SetNone(PyExc_MemoryError);
    return NULL;
  }

  /* execute the search. */
  if (!search_execute(self, x)) {
    PyErr_SetString(PyExc_RuntimeError, "failed to execute search");
    vector_free(x);
    return NULL;
  }

  /* cast the vector into a list. */
  PyObject *lst = PyList_FromVector(x);
  vector_free(x);
  if (!lst)
    return NULL;

  /* return the new list. */
  return lst;
}

/* --- */

/* Search_new(): allocation method for searches.
 */
static PyObject*
Search_new (PyTypeObject *type, PyObject *args, PyObject *kwargs) {
  /* allocate a new search. */
  Search *self = (Search*) type->tp_alloc(type, 0);
  if (!self)
    return NULL;

  /* initialize the model, dataset and grid. */
  self->grid = NULL;
  self->mdl = NULL;
  self->dat = NULL;

  /* initialize the buffer sizes. */
  self->D = self->P = self->K = self->N = self->n = 0;

#ifdef __VFL_USE_OPENCL
  /* initialize the opencl variables. */
  self->plat = NULL;
  self->dev = NULL;
  self->ctx = NULL;
  self->queue = NULL;
  self->prog = NULL;
  self->kern = NULL;
  self->src = NULL;

  /* initialize the device-side pointers. */
  self->dev_par = self->dev_var = self->dev_xgrid = NULL;
  self->dev_xdat = self->dev_pdat = self->dev_C = NULL;
  self->dev_cblk = NULL;

  /* initialize the host-side pointers. */
  self->par = self->var = NULL;
  self->xgrid = self->xmax = self->xdat = NULL;
  self->C = NULL;
  self->pdat = NULL;
#else
  self->cs = NULL;
#endif
  self->cov = NULL;
  self->vmax = 0.0;

  /* return the new object. */
  return (PyObject*) self;
}

/* Search_dealloc(): deallocation method for searches.
 */
static void
Search_dealloc (Search *self) {
  /* free the the kernel and calculation buffers. */
  free_buffers(self);
  free_kernel(self);

  /* release the object memory. */
  Py_TYPE(self)->tp_free((PyObject*) self);
}

/* Search_repr(): representation method for searches.
 */
static PyObject*
Search_repr (Search *self) {
  /* build and return the representation string. */
  return PyUnicode_FromFormat("<vfl.Search at 0x%x>", (long) self);
}

/* Search_getset: property definition structure for searches.
 */
static PyGetSetDef Search_getset[] = {
  { "model",
    (getter) Search_get_model,
    (setter) Search_set_model,
    Search_getset_model_doc,
    NULL
  },
  { "data",
    (getter) Search_get_data,
    (setter) Search_set_data,
    Search_getset_data_doc,
    NULL
  },
  { "grid",
    (getter) Search_get_grid,
    (setter) Search_set_grid,
    Search_getset_grid_doc,
    NULL
  },
  { "outputs",
    (getter) Search_get_outputs,
    (setter) Search_set_outputs,
    Search_getset_outputs_doc,
    NULL
  },
  { NULL }
};

/* Search_methods: method definition structure for searches.
 */
static PyMethodDef Search_methods[] = {
  { "execute",
    (PyCFunction) Search_method_execute,
    METH_VARARGS,
    Search_method_execute_doc
  },
  { NULL }
};

/* Search_Type: type definition structure for searches.
 */
PyTypeObject Search_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "vfl.Search",                                  /* tp_name           */
  sizeof(Search),                                /* tp_basicsize      */
  0,                                             /* tp_itemsize       */
  (destructor) Search_dealloc,                   /* tp_dealloc        */
  0,                                             /* tp_print          */
  0,                                             /* tp_getattr        */
  0,                                             /* tp_setattr        */
  0,                                             /* tp_reserved       */
  (reprfunc) Search_repr,                        /* tp_repr           */
  0,                                             /* tp_as_number      */
  0,                                             /* tp_as_sequence    */
  0,                                             /* tp_as_mapping     */
  0,                                             /* tp_hash           */
  0,                                             /* tp_call           */
  (reprfunc) Search_repr,                        /* tp_str            */
  0,                                             /* tp_getattro       */
  0,                                             /* tp_setattro       */
  0,                                             /* tp_as_buffer      */
  Py_TPFLAGS_DEFAULT |
  Py_TPFLAGS_BASETYPE,                           /* tp_flags          */
  Search_doc,                                    /* tp_doc            */
  0,                                             /* tp_traverse       */
  0,                                             /* tp_clear          */
  0,                                             /* tp_richcompare    */
  0,                                             /* tp_weaklistoffset */
  0,                                             /* tp_iter           */
  0,                                             /* tp_iternext       */
  Search_methods,                                /* tp_methods        */
  0,                                             /* tp_members        */
  Search_getset,                                 /* tp_getset         */
  0,                                             /* tp_base           */
  0,                                             /* tp_dict           */
  0,                                             /* tp_descr_get      */
  0,                                             /* tp_descr_set      */
  0,                                             /* tp_dictoffset     */
  vfl_base_init,                                 /* tp_init           */
  0,                                             /* tp_alloc          */
  Search_new                                     /* tp_new            */
};

/* Search_Type_init(): type initialization function for searches.
 */
int
Search_Type_init (PyObject *mod) {
  /* finalize the type object. */
  if (PyType_Ready(&Search_Type) < 0)
    return -1;

  /* take a reference to the type and add it to the module. */
  Py_INCREF(&Search_Type);
  PyModule_AddObject(mod, "Search", (PyObject*) &Search_Type);

  /* return success. */
  return 0;
}

