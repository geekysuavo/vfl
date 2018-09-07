
/* include the vfl header. */
#include <vfl/vfl.h>
#include <vfl/factor/product.h>

/* define documentation strings: */

PyDoc_STRVAR(
  Factor_doc,
"Factor() -> Factor object\n"
"\n");

PyDoc_STRVAR(
  Factor_getset_dims_doc,
"Number of dimensions of input space (read-only)\n"
"\n");

PyDoc_STRVAR(
  Factor_getset_parms_doc,
"Number of variational parameters (read-only)\n"
"\n");

PyDoc_STRVAR(
  Factor_getset_weights_doc,
"Number of linear weights (read-only)\n"
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

/* Factor_get_dims(): method for getting factor dimension counts.
 */
static PyObject*
Factor_get_dims (Factor *self) {
  /* return the value as an integer. */
  return PyLong_FromSize_t(factor_dims(self));
}

/* Factor_get_parms(): method for getting factor parameter counts.
 */
static PyObject*
Factor_get_parms (Factor *self) {
  /* return the value as an integer. */
  return PyLong_FromSize_t(factor_parms(self));
}

/* Factor_get_weights(): method for getting factor weight counts.
 */
static PyObject*
Factor_get_weights (Factor *self) {
  /* return the value as an integer. */
  return PyLong_FromSize_t(factor_weights(self));
}

/* Factor_get_dim(): method for getting factor dimension indices.
 */
static PyObject*
Factor_get_dim (Factor *self) {
  /* return the dimension index as an integer. */
  return PyLong_FromSize_t(self->d);
}

/* Factor_set_dim(): method for setting factor dimension indices.
 */
static int
Factor_set_dim (Factor *self, PyObject *value, void *closure) {
  /* get the new value. */
  const size_t d = PyLong_AsSize_t(value);
  if (PyErr_Occurred())
    return -1;

  /* set the dimension index and return success. */
  self->d = d;
  return 0;
}

/* Factor_get_fixed(): method for getting factor fixed flags.
 */
static PyObject*
Factor_get_fixed (Factor *self) {
  /* return the fixed flag as a boolean. */
  return PyBool_FromLong(self->fixed);
}

/* Factor_set_fixed(): method for setting factor fixed flags.
 */
static int
Factor_set_fixed (Factor *self, PyObject *value, void *closure) {
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

#define __max(a,b) ((a) > (b) ? (a) : (b))

/* Factor_mul(): multiplication operator for factors.
 */
static PyObject*
Factor_mul (PyObject *lhs, PyObject *rhs) {
  /* output object. */
  PyObject *obj = NULL;

  /* check if either operand is already a product. */
  if (Product_Check(lhs) && Product_Check(rhs)) { /* Product * Product. */
    /* allocate a new pre-sized product. */
    obj = product_new_with_size(
            Product_GET_SIZE(lhs) + Product_GET_SIZE(rhs),
            __max(Factor_MAX_DIM(lhs), Factor_MAX_DIM(rhs)),
            Factor_GET_PARMS(lhs) + Factor_GET_PARMS(rhs),
            __max(Factor_GET_WEIGHTS(lhs), Factor_GET_WEIGHTS(rhs)));

    /* check for allocation failure. */
    if (!obj)
      return NULL;

    /* store the left-hand factors. */
    size_t i = 0;
    for (size_t j = 0; j < Product_GET_SIZE(lhs); j++, i++)
      Product_SET_ITEM(obj, i, Product_GET_ITEM(lhs, j));

    /* store the right-hand factors. */
    for (size_t j = 0; j < Product_GET_SIZE(rhs); j++, i++)
      Product_SET_ITEM(obj, i, Product_GET_ITEM(rhs, j));
  }
  else if (Product_Check(lhs) && Factor_Check(rhs)) { /* Product * Factor */
    /* allocate a new pre-sized product. */
    obj = product_new_with_size(Product_GET_SIZE(lhs) + 1,
            __max(Factor_MAX_DIM(lhs), Factor_MAX_DIM(rhs)),
            Factor_GET_PARMS(lhs) + Factor_GET_PARMS(rhs),
            __max(Factor_GET_WEIGHTS(lhs), Factor_GET_WEIGHTS(rhs)));

    /* check for allocation failure. */
    if (!obj)
      return NULL;

    /* store the factors. */
    Product_SET_ITEM(obj, Product_GET_SIZE(obj) - 1, rhs);
    for (size_t j = 0; j < Product_GET_SIZE(lhs); j++)
      Product_SET_ITEM(obj, j, Product_GET_ITEM(lhs, j));
  }
  else if (Factor_Check(lhs) && Product_Check(rhs)) { /* Factor * Product */
    /* allocate a new pre-sized product. */
    obj = product_new_with_size(Product_GET_SIZE(rhs) + 1,
            __max(Factor_MAX_DIM(lhs), Factor_MAX_DIM(rhs)),
            Factor_GET_PARMS(lhs) + Factor_GET_PARMS(rhs),
            __max(Factor_GET_WEIGHTS(lhs), Factor_GET_WEIGHTS(rhs)));

    /* check for allocation failure. */
    if (!obj)
      return NULL;

    /* store the factors. */
    Product_SET_ITEM(obj, 0, lhs);
    for (size_t j = 0; j < Product_GET_SIZE(rhs); j++)
      Product_SET_ITEM(obj, j + 1, Product_GET_ITEM(rhs, j));
  }
  else if (Factor_Check(lhs) && Factor_Check(rhs)) { /* Factor * Factor. */
    /* allocate a new pre-sized product. */
    obj = product_new_with_size(2,
            __max(Factor_MAX_DIM(lhs), Factor_MAX_DIM(rhs)),
            Factor_GET_PARMS(lhs) + Factor_GET_PARMS(rhs),
            __max(Factor_GET_WEIGHTS(lhs), Factor_GET_WEIGHTS(rhs)));

    /* check for allocation failure. */
    if (!obj)
      return NULL;

    /* store the two factors. */
    Product_SET_ITEM(obj, 0, lhs);
    Product_SET_ITEM(obj, 1, rhs);
  }

  /* none of the above type combinations matched. */
  if (!obj)
    PyErr_SetNone(PyExc_TypeError);

  /* return the object, or null. */
  return obj;
}

#undef __max

/* --- */

/* Factor_method_mean(): compute the mean value of a factor.
 */
static PyObject*
Factor_method_mean (Factor *self, PyObject *args) {
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

/* Factor_method_var(): compute the variance of a factor.
 */
static PyObject*
Factor_method_var (Factor *self, PyObject *args) {
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

/* Factor_method_cov(): compute the covariance of a factor.
 */
static PyObject*
Factor_method_cov (Factor *self, PyObject *args) {
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

/* Factor_method_div(): compute the divergence to another factor.
 */
static PyObject*
Factor_method_div (Factor *self, PyObject *args) {
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

/* Factor_new(): allocation method for factors.
 */
static PyObject*
Factor_new (PyTypeObject *type, PyObject *args, PyObject *kwargs) {
  /* allocate a new factor. */
  Factor *self = (Factor*) type->tp_alloc(type, 0);
  Factor_reset(self);
  if (!self)
    return NULL;

  /* if the factor has an init function assigned, call it. */
  if (self->init && !self->init(self))
    return NULL;

  /* initialize and return the new object. */
  return (PyObject*) self;
}

/* Factor_dealloc(): deallocation method for factors.
 */
static void
Factor_dealloc (Factor *self) {
  /* if the factor has a free function assigned, call it. */
  if (self->free)
    self->free(self);

  /* free the information matrix and parameter vector. */
  matrix_free(self->inf);
  vector_free(self->par);

  /* release the object memory. */
  Py_TYPE(self)->tp_free((PyObject*) self);
}

/* Factor_repr(): representation method for factors.
 */
static PyObject*
Factor_repr (Factor *self) {
  /* build and return the representation string. */
  return PyUnicode_FromFormat("<%s at 0x%x>", Py_TYPE(self)->tp_name,
                              (long) self);
}

/* Factor_call(): evaluation method for factors.
 */
static PyObject*
Factor_call (Factor *self, PyObject *args, PyObject *kwargs) {
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

/* Factor_number: numeric operator definition structure for factors.
 */
static PyNumberMethods Factor_number = {
  NULL,                                     /* nb_add                     */
  NULL,                                     /* nb_subtract                */
  (binaryfunc) Factor_mul,                  /* nb_multiply                */
  NULL,                                     /* nb_remainder               */
  NULL,                                     /* nb_divmod                  */
  NULL,                                     /* nb_power                   */
  NULL,                                     /* nb_negative                */
  NULL,                                     /* nb_positive                */
  NULL,                                     /* nb_absolute                */
  NULL,                                     /* nb_bool                    */
  NULL,                                     /* nb_invert                  */
  NULL,                                     /* nb_lshift                  */
  NULL,                                     /* nb_rshift                  */
  NULL,                                     /* nb_and                     */
  NULL,                                     /* nb_xor                     */
  NULL,                                     /* nb_or                      */
  NULL,                                     /* nb_int                     */
  NULL,
  NULL,                                     /* nb_float                   */
  NULL,                                     /* nb_inplace_add             */
  NULL,                                     /* nb_inplace_subtract        */
  NULL,                                     /* nb_inplace_multiply        */
  NULL,                                     /* nb_inplace_remainder       */
  NULL,                                     /* nb_inplace_power           */
  NULL,                                     /* nb_inplace_lshift          */
  NULL,                                     /* nb_inplace_rshift          */
  NULL,                                     /* nb_inplace_and             */
  NULL,                                     /* nb_inplace_xor             */
  NULL,                                     /* nb_inplace_or              */
  NULL,                                     /* nb_floor_divide            */
  NULL,                                     /* nb_true_divide             */
  NULL,                                     /* nb_inplace_floor_divide    */
  NULL,                                     /* nb_inplace_true_divide     */
  NULL,                                     /* nb_index                   */
  NULL,                                     /* nb_matrix_multiply         */
  NULL,                                     /* nb_inplace_matrix_multiply */
};

/* Factor_getset: property definition structure for factors.
 */
static PyGetSetDef Factor_getset[] = {
  { "dims",
    (getter) Factor_get_dims,
    NULL,
    Factor_getset_dims_doc,
    NULL
  },
  { "parms",
    (getter) Factor_get_parms,
    NULL,
    Factor_getset_parms_doc,
    NULL
  },
  { "weights",
    (getter) Factor_get_weights,
    NULL,
    Factor_getset_weights_doc,
    NULL
  },
  { "dim",
    (getter) Factor_get_dim,
    (setter) Factor_set_dim,
    Factor_getset_dim_doc,
    NULL
  },
  { "fixed",
    (getter) Factor_get_fixed,
    (setter) Factor_set_fixed,
    Factor_getset_fixed_doc,
    NULL
  },
  { NULL }
};

/* Factor_methods: method definition structure for factors.
 */
static PyMethodDef Factor_methods[] = {
  { "mean",
    (PyCFunction) Factor_method_mean,
    METH_VARARGS,
    Factor_method_mean_doc
  },
  { "var",
    (PyCFunction) Factor_method_var,
    METH_VARARGS,
    Factor_method_var_doc
  },
  { "cov",
    (PyCFunction) Factor_method_cov,
    METH_VARARGS,
    Factor_method_cov_doc
  },
  { "div",
    (PyCFunction) Factor_method_div,
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
  (destructor) Factor_dealloc,                   /* tp_dealloc        */
  0,                                             /* tp_print          */
  0,                                             /* tp_getattr        */
  0,                                             /* tp_setattr        */
  0,                                             /* tp_reserved       */
  (reprfunc) Factor_repr,                        /* tp_repr           */
  &Factor_number,                                /* tp_as_number      */
  0,                                             /* tp_as_sequence    */
  0,                                             /* tp_as_mapping     */
  0,                                             /* tp_hash           */
  (ternaryfunc) Factor_call,                     /* tp_call           */
  (reprfunc) Factor_repr,                        /* tp_str            */
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
  Factor_new                                     /* tp_new            */
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

