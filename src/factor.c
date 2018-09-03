
/* include the vfl header. */
#include <vfl/vfl.h>

/* define documentation strings: */

PyDoc_STRVAR(
  Factor_doc,
"Factor() -> Factor object\n"
"\n");

PyDoc_STRVAR(
  Factor_getset_dim_doc,
"Dimension index, for univariate factors (read/write)\n"
"\n");

PyDoc_STRVAR(
  Factor_getset_fixed_doc,
"Parameter mutability flag (read/write)\n"
"\n");

PyDoc_STRVAR(
  Factor_method_mean_doc,
"Mean of a factor at a given input.\n"
"\n");

PyDoc_STRVAR(
  Factor_method_var_doc,
"Variance of a factor at a given input.\n"
"\n");

PyDoc_STRVAR(
  Factor_method_cov_doc,
"Covariance of a factor at two inputs.\n"
"\n");

PyDoc_STRVAR(
  Factor_method_div_doc,
"Kullback-Liebler divergence from another factor.\n"
"\n");

/* factor_get_dim(): method for getting factor dimension indices.
 */
static PyObject*
factor_get_dim (Factor *self) {
  /* return the dimension index as an integer. */
  return PyLong_FromSize_t(self->d);
}

/* factor_set_dim(): method for setting factor dimension indices.
 */
static int
factor_set_dim (Factor *self, PyObject *value, void *closure) {
  /* get the new value. */
  const size_t d = PyLong_AsSize_t(value);
  if (PyErr_Occurred())
    return -1;

  /* set the dimension index and return success. */
  self->d = d;
  return 0;
}

/* factor_get_fixed(): method for getting factor fixed flags.
 */
static PyObject*
factor_get_fixed (Factor *self) {
  /* return the fixed flag as a boolean. */
  return PyBool_FromLong(self->fixed);
}

/* factor_set_fixed(): method for setting factor fixed flags.
 */
static int
factor_set_fixed (Factor *self, PyObject *value, void *closure) {
  /* check that the value is a boolean. */
  if (!PyBool_Check(value)) {
    PyErr_SetString(PyExc_TypeError, "'fixed' expects bool");
    return -1;
  }

  /* set the fixed flag and return success. */
  self->fixed = (int) PyLong_AsLong(value);
  return 0;
}

/* --- */

/* factor_method_mean(): compute the mean value of a factor.
 */
static PyObject*
factor_method_mean (Factor *self, PyObject *args) {
  /* return nothing if the factor doesn't have a mean method. */
  if (!self->mean)
    Py_RETURN_NONE;

  /* parse a datum and an optional basis index. */
  Datum *dat = NULL;
  size_t i = 0;
  if (!PyArg_ParseTuple(args, "O!|O&", &Datum_Type, &dat,
                        PySize_t_Converter, &i))
                          return NULL;

  /* call the mean function and return the result. */
  return PyFloat_FromDouble(self->mean(self, dat->x, dat->p, i));
}

/* factor_method_var(): compute the variance of a factor.
 */
static PyObject*
factor_method_var (Factor *self, PyObject *args) {
  /* return nothing if the factor doesn't have a variance method. */
  if (!self->var)
    Py_RETURN_NONE;

  /* parse a datum and two optional basis indices. */
  Datum *dat = NULL;
  size_t i = 0, j = 0;
  if (!PyArg_ParseTuple(args, "O!|O&O&", &Datum_Type, &dat,
                        PySize_t_Converter, &i,
                        PySize_t_Converter, &j))
                          return NULL;

  /* call the variance function and return the result. */
  return PyFloat_FromDouble(self->var(self, dat->x, dat->p, i, j));
}

/* factor_method_cov(): compute the covariance of a factor.
 */
static PyObject*
factor_method_cov (Factor *self, PyObject *args) {
  /* return nothing is the factor doesn't have a covariance method. */
  if (!self->cov)
    Py_RETURN_NONE;

  /* parse two datum objects. */
  Datum *d1 = NULL, *d2 = NULL;
  if (!PyArg_ParseTuple(args, "O!O!",
                        &Datum_Type, &d1,
                        &Datum_Type, &d2))
                          return NULL;

  /* call the covariance function and return the result. */
  return PyFloat_FromDouble(self->cov(self, d1->x, d2->x, d1->p, d2->p));
}

/* factor_method_div(): compute the divergence to another factor.
 */
static PyObject*
factor_method_div (Factor *self, PyObject *args) {
  /* return nothing if the factor doesn't have a divergence method. */
  if (!self->div)
    Py_RETURN_NONE;

  /* parse a second factor of the same type. */
  Factor *f = NULL;
  if (!PyArg_ParseTuple(args, "O!", Py_TYPE(self), &f))
    return NULL;

  /* call the divergence function and return the result. */
  return PyFloat_FromDouble(self->div(self, f));
}

/* --- */

/* factor_new(): allocation method for factors.
 */
static PyObject*
factor_new (PyTypeObject *type, PyObject *args, PyObject *kwargs) {
  /* allocate a new factor. */
  Factor *self = (Factor*) type->tp_alloc(type, 0);
  factor_reset(self);
  if (!self)
    return NULL;

  /* if the factor has an init function assigned, call it. */
  if (self->init && !self->init(self))
    return 0;

  /* initialize and return the new object. */
  return (PyObject*) self;
}

/* factor_dealloc(): deallocation method for factors.
 */
static void
factor_dealloc (Factor *self) {
  /* if the factor has a free function assigned, call it. */
  if (self->free)
    self->free(self);

  /* free the information matrix and parameter vector. */
  matrix_free(self->inf);
  vector_free(self->par);

  /* release the object memory. */
  Py_TYPE(self)->tp_free((PyObject*) self);
}

/* factor_repr(): representation method for factors.
 */
static PyObject*
factor_repr (Factor *self) {
  /* build and return the representation string. */
  return PyUnicode_FromFormat("<%s at 0x%x>", Py_TYPE(self)->tp_name,
                              (long) self);
}

/* factor_call(): evaluation method for factors.
 */
static PyObject*
factor_call (Factor *self, PyObject *args, PyObject *kwargs) {
  /* return nothing if the factor doesn't have an evaluation method. */
  if (!self->eval)
    Py_RETURN_NONE;

  /* parse a datum and an optional basis index. */
  Datum *dat = NULL;
  size_t i = 0;
  if (!PyArg_ParseTuple(args, "O!|O&", &Datum_Type, &dat,
                        PySize_t_Converter, &i))
                          return NULL;

  /* evaluate the factor and return the result. */
  return PyFloat_FromDouble(self->eval(self, dat->x, dat->p, i));
}

/* Factor_getset: property definition structure for factors.
 */
static PyGetSetDef Factor_getset[] = {
  { "dim",
    (getter) factor_get_dim,
    (setter) factor_set_dim,
    Factor_getset_dim_doc,
    NULL
  },
  { "fixed",
    (getter) factor_get_fixed,
    (setter) factor_set_fixed,
    Factor_getset_fixed_doc,
    NULL
  },
  { NULL }
};

/* Factor_methods: method definition structure for factors.
 */
static PyMethodDef Factor_methods[] = {
  { "mean",
    (PyCFunction) factor_method_mean,
    METH_VARARGS,
    Factor_method_mean_doc
  },
  { "var",
    (PyCFunction) factor_method_var,
    METH_VARARGS,
    Factor_method_var_doc
  },
  { "cov",
    (PyCFunction) factor_method_cov,
    METH_VARARGS,
    Factor_method_cov_doc
  },
  { "div",
    (PyCFunction) factor_method_div,
    METH_VARARGS,
    Factor_method_div_doc
  },
  { NULL }
};

/* Factor_Type: type definition structure for factors.
 */
PyTypeObject Factor_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "vfl.Factor",                                  /* tp_name           */
  sizeof(Factor),                                /* tp_basicsize      */
  0,                                             /* tp_itemsize       */
  (destructor) factor_dealloc,                   /* tp_dealloc        */
  0,                                             /* tp_print          */
  0,                                             /* tp_getattr        */
  0,                                             /* tp_setattr        */
  0,                                             /* tp_reserved       */
  (reprfunc) factor_repr,                        /* tp_repr           */
  0,                                             /* tp_as_number      */
  0,                                             /* tp_as_sequence    */
  0,                                             /* tp_as_mapping     */
  0,                                             /* tp_hash           */
  (ternaryfunc) factor_call,                     /* tp_call           */
  (reprfunc) factor_repr,                        /* tp_str            */
  0,                                             /* tp_getattro       */
  0,                                             /* tp_setattro       */
  0,                                             /* tp_as_buffer      */
  Py_TPFLAGS_DEFAULT |
  Py_TPFLAGS_BASETYPE,                           /* tp_flags          */
  Factor_doc,                                    /* tp_doc            */
  0,                                             /* tp_traverse       */
  0,                                             /* tp_clear          */
  0,                                             /* tp_richcompare    */
  0,                                             /* tp_weaklistoffset */
  0,                                             /* tp_iter           */
  0,                                             /* tp_iternext       */
  Factor_methods,                                /* tp_methods        */
  0,                                             /* tp_members        */
  Factor_getset,                                 /* tp_getset         */
  0,                                             /* tp_base           */
  0,                                             /* tp_dict           */
  0,                                             /* tp_descr_get      */
  0,                                             /* tp_descr_set      */
  0,                                             /* tp_dictoffset     */
  vfl_base_init,                                 /* tp_init           */
  0,                                             /* tp_alloc          */
  factor_new                                     /* tp_new            */
};

/* Factor_Type_init(): type initialization function for factors.
 */
int
Factor_Type_init (PyObject *mod) {
  /* finalize the type object. */
  if (PyType_Ready(&Factor_Type) < 0)
    return -1;

  /* take a reference to the type and add it to the module. */
  Py_INCREF(&Factor_Type);
  PyModule_AddObject(mod, "Factor", (PyObject*) &Factor_Type);

  /* return success. */
  return 0;
}

