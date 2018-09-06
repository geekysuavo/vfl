
/* include the vfl header. */
#include <vfl/vfl.h>

/* define documentation strings: */

PyDoc_STRVAR(
  Model_doc,
"Model() -> Model object\n"
"\n");

PyDoc_STRVAR(
  Model_getset_dims_doc,
"Number of dimensions of input space (read-only)\n"
"\n");

PyDoc_STRVAR(
  Model_getset_parms_doc,
"Number of variational parameters (read-only)\n"
"\n");

PyDoc_STRVAR(
  Model_getset_weights_doc,
"Number of linear weights (read-only)\n"
"\n");

PyDoc_STRVAR(
  Model_getset_bound_doc,
"Value of the variational lower bound (read-only)\n"
"\n");

PyDoc_STRVAR(
  Model_getset_nu_doc,
"Prior noise/weight precision ratio (read/write)\n"
"\n");

PyDoc_STRVAR(
  Model_getset_wmean_doc,
"Weight mean parameters (read/write)\n"
"\n");

PyDoc_STRVAR(
  Model_getset_wcov_doc,
"Weight covariance parameters (read/write)\n"
"\n");

PyDoc_STRVAR(
  Model_getset_data_doc,
"Associated dataset (read/write)\n"
"\n");

PyDoc_STRVAR(
  Model_getset_factors_doc,
"Associated set of variational factors (read/write)\n"
"\n");

PyDoc_STRVAR(
  Model_getset_priors_doc,
"Associated set of factor priors (read-only)\n"
"\n");

PyDoc_STRVAR(
  Model_method_reset_doc,
"Reset a model to its a priori state.\n"
"\n");

PyDoc_STRVAR(
  Model_method_add_doc,
"Add one or more factors to a model.\n"
"\n");

PyDoc_STRVAR(
  Model_method_infer_doc,
"Infer the linear parameters of a model.\n"
"\n");

PyDoc_STRVAR(
  Model_method_mean_doc,
"Mean of a model at a given input.\n"
"\n");

PyDoc_STRVAR(
  Model_method_var_doc,
"Variance of a model at a given input.\n"
"\n");

PyDoc_STRVAR(
  Model_method_predict_doc,
"Predict multiple means and variances of a model.\n"
"\n");

/* Model_seq_len(): method for getting model factor counts.
 */
static Py_ssize_t
Model_seq_len (Model *self) {
  /* return the current factor count as an integer. */
  return (Py_ssize_t) self->M;
}

/* Model_seq_get(): method for getting model factors.
 */
static PyObject*
Model_seq_get (Model *self, Py_ssize_t i) {
  /* check that the index is in bounds. */
  const Py_ssize_t n = Model_seq_len(self);
  if (n == 0 || i < 0 || i >= n) {
    PyErr_SetNone(PyExc_IndexError);
    return NULL;
  }

  /* return a new reference to the factor. */
  Py_INCREF(self->factors[i]);
  return (PyObject*) self->factors[i];
}

/* Model_seq_set(): method for setting model factors.
 */
static int
Model_seq_set (Model *self, Py_ssize_t i, PyObject *v) {
  /* check that the index is in bounds. */
  const Py_ssize_t n = Model_seq_len(self);
  if (n == 0 || i < 0 || i >= n) {
    PyErr_SetNone(PyExc_IndexError);
    return -1;
  }

  /* check that the value is a factor. */
  if (!v || !Factor_Check(v)) {
    PyErr_SetNone(PyExc_TypeError);
    return -1;
  }

  /* attempt to place the factor into the model. */
  if (!model_set_factor(self, i, (Factor*) v)) {
    PyErr_SetString(PyExc_RuntimeError, "failed to set factor");
    return -1;
  }

  /* return success. */
  return 0;
}

/* --- */

/* Model_get_dims(): method to get model dimensionalities.
 */
static PyObject*
Model_get_dims (Model *self) {
  /* return the dimension count as an integer. */
  return PyLong_FromSize_t(self->D);
}

/* Model_get_parms(): method to get model parameter counts.
 */
static PyObject*
Model_get_parms (Model *self) {
  /* return the parameter count as an integer. */
  return PyLong_FromSize_t(self->P);
}

/* Model_get_weights(): method to get model weight counts.
 */
static PyObject*
Model_get_weights (Model *self) {
  /* return the weight count as an integer. */
  return PyLong_FromSize_t(self->K);
}

/* Model_get_bound(): method to get model lower bounds.
 */
static PyObject*
Model_get_bound (Model *self) {
  /* return the bound as a float. */
  return PyFloat_FromDouble(model_bound(self));
}

/* Model_get_nu(): method to get model precision ratios.
 */
static PyObject*
Model_get_nu (Model *mdl) {
  /* return the ratio parameter as a float. */
  return PyFloat_FromDouble(mdl->nu);
}

/* Model_set_nu(): method to set model precision ratios.
 */
static int
Model_set_nu (Model *mdl, PyObject *value, void *closure) {
  /* get the new value. */
  const double nu = PyFloat_AsDouble(value);
  if (PyErr_Occurred())
    return -1;

  /* set the noise/weight precision ratio. */
  if (!model_set_nu(mdl, nu)) {
    PyErr_SetString(PyExc_ValueError, "expected positive precision ratio");
    return -1;
  }

  /* return success. */
  return 0;
}

/* Model_get_wmean(): method to get model weight means.
 */
static PyObject*
Model_get_wmean (Model *self) {
  /* return the weight means as a list. */
  return PyList_FromVector(self->wbar);
}

/* Model_set_wmean(): method to set model weight means.
 */
static int
Model_set_wmean (Model *self, PyObject *value, void *closure) {
  /* get the new value. */
  Vector *wbar = PySequence_AsVector(value);
  if (!wbar)
    return -1;

  /* check that the vector has the correct length. */
  if (wbar->len != self->K) {
    PyErr_SetNone(PyExc_TypeError);
    vector_free(wbar);
    return -1;
  }

  /* copy the vector contents. */
  vector_copy(self->wbar, wbar);
  vector_free(wbar);

  /* return success. */
  return 0;
}

/* Model_get_wcov(): method to get model weight covariances.
 */
static PyObject*
Model_get_wcov (Model *self) {
  /* return the weight covariances as a list. */
  return PyList_FromMatrix(self->Sigma);
}

/* Model_set_wcov(): method to set model weight covariances.
 */
static int
Model_set_wcov (Model *self, PyObject *value, void *closure) {
  /* get the new value. */
  Matrix *Sigma = PySequence_AsMatrix(value);
  if (!Sigma)
    return -1;

  /* check that the matrix has correct size. */
  if (Sigma->rows != self->K ||
      Sigma->cols != self->K) {
    PyErr_SetNone(PyExc_TypeError);
    matrix_free(Sigma);
    return -1;
  }

  /* allocate a matrix to store the cholesky decomposition into. */
  Matrix *L = matrix_alloc(self->K, self->K);
  if (!L) {
    PyErr_SetNone(PyExc_MemoryError);
    matrix_free(Sigma);
    return -1;
  }

  /* check that the matrix is positive definite. */
  matrix_copy(L, Sigma);
  if (!chol_decomp(L)) {
    PyErr_SetString(PyExc_RuntimeError, "matrix is not positive definite");
    matrix_free(Sigma);
    matrix_free(L);
    return -1;
  }

  /* copy the matrix and its decomposition. */
  matrix_copy(self->Sigma, Sigma);
  matrix_copy(self->L, L);

  /* compute and copy the matrix inverse. */
  chol_invert(L, Sigma);
  matrix_copy(self->Sinv, Sigma);

  /* free the temporary matrices. */
  matrix_free(Sigma);
  matrix_free(L);

  /* return success. */
  return 0;
}

/* Model_get_data(): method to get model datasets.
 */
static PyObject*
Model_get_data (Model *self) {
  /* return nothing if the dataset is null. */
  if (!self->dat)
    Py_RETURN_NONE;

  /* return a new reference to the dataset. */
  Py_INCREF(self->dat);
  return (PyObject*) self->dat;
}

/* Model_set_data(): method to set model datasets.
 */
static int
Model_set_data (Model *self, PyObject *value, void *closure) {
  /* check that the value is a dataset. */
  if (!value || !Data_Check(value)) {
    PyErr_SetString(PyExc_TypeError, "expected Data object");
    return -1;
  }

  /* attempt to set the new dataset. */
  if (!model_set_data(self, (Data*) value)) {
    PyErr_SetString(PyExc_RuntimeError, "failed to set data");
    return -1;
  }

  /* return success. */
  return 0;
}

/* Model_get_factors(): method to get model factor lists.
 */
static PyObject*
Model_get_factors (Model *self) {
  /* create a tuple with the necessary length. */
  const Py_ssize_t m = Model_seq_len(self);
  PyObject *tup = PyTuple_New(m);
  if (!tup)
    return NULL;

  /* add the factors into the new tuple. */
  for (Py_ssize_t i = 0; i < m; i++) {
    Py_INCREF(self->factors[i]);
    PyTuple_SET_ITEM(tup, i, (PyObject*) self->factors[i]);
  }

  /* return the new tuple. */
  return tup;
}

/* Model_set_factors(): method to set model factor lists.
 */
static int
Model_set_factors (Model *self, PyObject *value, void *closure) {
  /* check that the value is non-null. */
  if (!value) {
    PyErr_SetNone(PyExc_AttributeError);
    return -1;
  }

  /* accept either factors or sequences of factors. */
  if (Factor_Check(value)) {
    /* clear the factors from the model and add the new factor. */
    if (!model_clear_factors(self) ||
        !model_add_factor(self, (Factor*) value)) {
      PyErr_SetString(PyExc_RuntimeError, "failed to add factor");
      return -1;
    }
  }
  else if (PySequence_Check(value)) {
    /* check that the sequence has at least one element. */
    const Py_ssize_t n = PySequence_Length(value);
    if (n < 1) {
      PyErr_SetString(PyExc_ValueError, "one or more factors required");
      return -1;
    }

    /* check that every element is a factor. */
    int chk = 1;
    for (Py_ssize_t i = 0; i < n; i++) {
      PyObject *elem = PySequence_GetItem(value, i);
      chk &= (elem && Factor_Check(elem));
      Py_XDECREF(elem);
    }

    /* fail if any element is not a factor. */
    if (!chk) {
      PyErr_SetString(PyExc_TypeError, "expected sequence of factors");
      return -1;
    }

    /* clear the factors from the model. */
    if (!model_clear_factors(self)) {
      PyErr_SetString(PyExc_RuntimeError, "failed to clear all factors");
      return -1;
    }

    /* add each factor into the model. */
    for (Py_ssize_t i = 0; i < n; i++) {
      Factor *f = (Factor*) PySequence_GetItem(value, i);
      Py_DECREF(f);

      if (!model_add_factor(self, f)) {
        PyErr_SetString(PyExc_RuntimeError, "failed to add factor");
        return -1;
      }
    }
  }
  else {
    /* neither a factor nor a sequence... */
    PyErr_SetNone(PyExc_TypeError);
    return -1;
  }

  /* return success. */
  return 0;
}

/* Model_get_priors(): method to get model prior factor lists.
 */
static PyObject*
Model_get_priors (Model *self) {
  /* create a tuple with the necessary length. */
  const Py_ssize_t m = Model_seq_len(self);
  PyObject *tup = PyTuple_New(m);
  if (!tup)
    return NULL;

  /* add the prior factors into the new tuple. */
  for (Py_ssize_t i = 0; i < m; i++) {
    Py_INCREF(self->priors[i]);
    PyTuple_SET_ITEM(tup, i, (PyObject*) self->priors[i]);
  }

  /* return the new tuple. */
  return tup;
}

/* --- */

/* Model_method_reset(): reset a model to its a priori state.
 */
static PyObject*
Model_method_reset (Model *self, PyObject *args) {
  /* reset the model and return nothing. */
  model_reset(self);
  Py_RETURN_NONE;
}

/* Model_method_add(): add one or more factors to a model.
 */
static PyObject*
Model_method_add (Model *self, PyObject *args) {
  /* check for a minimum number of variables. */
  if (!args || !PyTuple_Check(args) || PyTuple_Size(args) < 1) {
    PyErr_SetNone(PyExc_ValueError);
    return NULL;
  }

  /* loop over the arguments. */
  const Py_ssize_t n = PyTuple_GET_SIZE(args);
  for (Py_ssize_t i = 0; i < n; i++) {
    /* get the current argument. */
    PyObject *arg = PyTuple_GET_ITEM(args, i);

    /* check that the argument is a factor. */
    if (!Factor_Check(arg)) {
      PyErr_SetNone(PyExc_TypeError);
      return NULL;
    }

    /* add the factor to the model. */
    if (!model_add_factor(self, (Factor*) arg)) {
      PyErr_SetString(PyExc_RuntimeError, "failed to add factor");
      return NULL;
    }
  }

  /* return nothing. */
  Py_RETURN_NONE;
}

/* Model_method_infer(): infer the weights of a model.
 */
static PyObject*
Model_method_infer (Model *self, PyObject *args) {
  /* run inference and return nothing. */
  model_infer(self);
  Py_RETURN_NONE;
}

/* Model_method_mean(): compute the mean value of a model.
 */
static PyObject*
Model_method_mean (Model *self, PyObject *args) {
  /* return nothing if the model has no prediction function. */
  if (!self->predict)
    Py_RETURN_NONE;

  /* declare variables for parsing arguments. */
  Data *dat = NULL;
  Datum *d = NULL;

  /* first try: parse a datum. */
  if (PyArg_ParseTuple(args, "O!", &Datum_Type, &d)) {
    double mean = 0.0, var = 0.0;
    if (model_predict(self, d->x, d->p, &mean, &var))
      return PyFloat_FromDouble(mean);
    else {
      PyErr_SetString(PyExc_RuntimeError, "datum dimension mismatch");
      return NULL;
    }
  }

  /* second try: parse a dataset. */
  PyErr_Clear();
  if (PyArg_ParseTuple(args, "O!", &Data_Type, &dat)) {
    if (model_predict_all(self, dat, NULL))
      Py_RETURN_NONE;
    else {
      PyErr_SetString(PyExc_RuntimeError, "data dimension mismatch");
      return NULL;
    }
  }

  /* invalid arguments. */
  return NULL;
}

/* Model_method_var(): compute the variance of a model.
 */
static PyObject*
Model_method_var (Model *self, PyObject *args) {
  /* return nothing if the model has no prediction function. */
  if (!self->predict)
    Py_RETURN_NONE;

  /* declare variables for parsing arguments. */
  Data *dat = NULL;
  Datum *d = NULL;

  /* first try: parse a datum. */
  if (PyArg_ParseTuple(args, "O!", &Datum_Type, &d)) {
    double mean = 0.0, var = 0.0;
    if (model_predict(self, d->x, d->p, &mean, &var))
      return PyFloat_FromDouble(var);
    else {
      PyErr_SetString(PyExc_RuntimeError, "datum dimension mismatch");
      return NULL;
    }
  }

  /* second try: parse a dataset. */
  PyErr_Clear();
  if (PyArg_ParseTuple(args, "O!", &Data_Type, &dat)) {
    if (model_predict_all(self, NULL, dat))
      Py_RETURN_NONE;
    else {
      PyErr_SetString(PyExc_RuntimeError, "data dimension mismatch");
      return NULL;
    }
  }

  /* invalid arguments. */
  return NULL;
}

/* Model_method_predict(): compute a set of model predictions.
 */
static PyObject*
Model_method_predict (Model *self, PyObject *args, PyObject *kwargs) {
  /* return nothing if the model has no prediction function. */
  if (!self->predict)
    Py_RETURN_NONE;

  /* declare variables for parsing arguments. */
  Data *mean = NULL, *var = NULL;

  /* parse the datasets. */
  static char *kwlist[] = { "mean", "var", NULL };
  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|$O!O!", kwlist,
                                   &Data_Type, &mean,
                                   &Data_Type, &var))
                                     return NULL;

  /* execute the prediction. */
  if (!model_predict_all(self, mean, var)) {
    PyErr_SetString(PyExc_RuntimeError, "failed to compute predictions");
    return NULL;
  }

  /* return nothing. */
  Py_RETURN_NONE;
}

/* --- */

/* Model_new(): allocation method for models.
 */
static PyObject*
Model_new (PyTypeObject *type, PyObject *args, PyObject *kwargs) {
  /* allocate a new model. */
  Model *self = (Model*) type->tp_alloc(type, 0);
  Model_reset(self);
  if (!self)
    return NULL;

  /* if the model has an init function assigned, call it. */
  if (self->init && !self->init(self))
    return NULL;

  /* return the new object. */
  return (PyObject*) self;
}

/* Model_dealloc(): deallocation method for models.
 */
static void
Model_dealloc (Model *self) {
  /* free the weight means and covariances. */
  vector_free(self->wbar);
  matrix_free(self->Sigma);

  /* free the logistic parameters. */
  vector_free(self->xi);

  /* free the intermediates. */
  matrix_free(self->Sinv);
  matrix_free(self->L);
  vector_free(self->h);

  /* release the reference to the associated dataset. */
  Py_XDECREF(self->dat);

  /* release the references to the associated factors. */
  for (size_t i = 0; i < self->M; i++) {
    Py_XDECREF(self->factors[i]);
    Py_XDECREF(self->priors[i]);
  }

  /* free the factor array.
   * (factors and priors are are contiguous block)
   */
  free(self->factors);

  /* free the temporary vector. */
  vector_free(self->tmp);

  /* release the object memory. */
  Py_TYPE(self)->tp_free((PyObject*) self);
}

/* Model_repr(): representation function for models.
 */
static PyObject*
Model_repr (Model *self) {
  /* build and return the representation string. */
  return PyUnicode_FromFormat("<%s at 0x%x>", Py_TYPE(self)->tp_name,
                              (long) self);
}

/* Model_call(): evaluation method for models.
 */
static PyObject*
Model_call (Model *self, PyObject *args, PyObject *kwargs) {
  /* declare variables for parsing arguments. */
  Data *dat = NULL;
  Datum *d = NULL;

  /* first try: parse a datum. */
  if (PyArg_ParseTuple(args, "O!", &Datum_Type, &d)) {
    return PyFloat_FromDouble(model_eval(self, d->x, d->p));
  }

  /* second try: parse a dataset. */
  PyErr_Clear();
  if (PyArg_ParseTuple(args, "O!", &Data_Type, &dat)) {
    if (!model_eval_all(self, dat)) {
      PyErr_SetString(PyExc_RuntimeError, "data dimension mismatch");
      return NULL;
    }

    Py_RETURN_NONE;
  }

  /* invalid arguments. */
  return NULL;
}

/* Model_sequence: sequence definition structure for models.
 */
static PySequenceMethods Model_sequence = {
  (lenfunc) Model_seq_len,                       /* sq_length         */
  NULL,                                          /* sq_concat         */
  NULL,                                          /* sq_repeat         */
  (ssizeargfunc) Model_seq_get,                  /* sq_item           */
  NULL,
  (ssizeobjargproc) Model_seq_set,               /* sq_ass_item       */
  NULL,
  NULL,                                          /* sq_contains       */
  NULL,                                          /* sq_inplace_concat */
  NULL                                           /* sq_inplace_repeat */
};

/* Model_getset: property definition structure for models.
 */
static PyGetSetDef Model_getset[] = {
  { "dims",
    (getter) Model_get_dims,
    NULL,
    Model_getset_dims_doc,
    NULL
  },
  { "parms",
    (getter) Model_get_parms,
    NULL,
    Model_getset_parms_doc,
    NULL
  },
  { "weights",
    (getter) Model_get_weights,
    NULL,
    Model_getset_weights_doc,
    NULL
  },
  { "bound",
    (getter) Model_get_bound,
    NULL,
    Model_getset_bound_doc,
    NULL
  },
  { "nu",
    (getter) Model_get_nu,
    (setter) Model_set_nu,
    Model_getset_nu_doc,
    NULL
  },
  { "wbar",
    (getter) Model_get_wmean,
    (setter) Model_set_wmean,
    Model_getset_wmean_doc,
    NULL
  },
  { "Sigma",
    (getter) Model_get_wcov,
    (setter) Model_set_wcov,
    Model_getset_wcov_doc,
    NULL
  },
  { "data",
    (getter) Model_get_data,
    (setter) Model_set_data,
    Model_getset_data_doc,
    NULL
  },
  { "factors",
    (getter) Model_get_factors,
    (setter) Model_set_factors,
    Model_getset_factors_doc,
    NULL
  },
  { "priors",
    (getter) Model_get_priors,
    NULL,
    Model_getset_priors_doc,
    NULL
  },
  { NULL }
};

/* Model_methods: method definition structure for models.
 */
static PyMethodDef Model_methods[] = {
  { "reset",
    (PyCFunction) Model_method_reset,
    METH_VARARGS,
    Model_method_reset_doc
  },
  { "add",
    (PyCFunction) Model_method_add,
    METH_VARARGS,
    Model_method_add_doc
  },
  { "infer",
    (PyCFunction) Model_method_infer,
    METH_VARARGS,
    Model_method_infer_doc
  },
  { "mean",
    (PyCFunction) Model_method_mean,
    METH_VARARGS,
    Model_method_mean_doc
  },
  { "var",
    (PyCFunction) Model_method_var,
    METH_VARARGS,
    Model_method_var_doc
  },
  { "predict",
    (PyCFunction) Model_method_predict,
    METH_VARARGS | METH_KEYWORDS,
    Model_method_predict_doc
  },
  { NULL }
};

/* Model_Type: type definition structure for models.
 */
PyTypeObject Model_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "vfl.Model",                                   /* tp_name           */
  sizeof(Model),                                 /* tp_basicsize      */
  0,                                             /* tp_itemsize       */
  (destructor) Model_dealloc,                    /* tp_dealloc        */
  0,                                             /* tp_print          */
  0,                                             /* tp_getattr        */
  0,                                             /* tp_setattr        */
  0,                                             /* tp_reserved       */
  (reprfunc) Model_repr,                         /* tp_repr           */
  0,                                             /* tp_as_number      */
  &Model_sequence,                               /* tp_as_sequence    */
  0,                                             /* tp_as_mapping     */
  0,                                             /* tp_hash           */
  (ternaryfunc) Model_call,                      /* tp_call           */
  (reprfunc) Model_repr,                         /* tp_str            */
  0,                                             /* tp_getattro       */
  0,                                             /* tp_setattro       */
  0,                                             /* tp_as_buffer      */
  Py_TPFLAGS_DEFAULT |
  Py_TPFLAGS_BASETYPE,                           /* tp_flags          */
  Model_doc,                                     /* tp_doc            */
  0,                                             /* tp_traverse       */
  0,                                             /* tp_clear          */
  0,                                             /* tp_richcompare    */
  0,                                             /* tp_weaklistoffset */
  0,                                             /* tp_iter           */
  0,                                             /* tp_iternext       */
  Model_methods,                                 /* tp_methods        */
  0,                                             /* tp_members        */
  Model_getset,                                  /* tp_getset         */
  0,                                             /* tp_base           */
  0,                                             /* tp_dict           */
  0,                                             /* tp_descr_get      */
  0,                                             /* tp_descr_set      */
  0,                                             /* tp_dictoffset     */
  vfl_base_init,                                 /* tp_init           */
  0,                                             /* tp_alloc          */
  Model_new                                      /* tp_new            */
};

/* Model_Type_init(): type initialization function for models.
 */
int
Model_Type_init (PyObject *mod) {
  /* finalize the type object. */
  if (PyType_Ready(&Model_Type) < 0)
    return -1;

  /* take a reference to the type and add it to the module. */
  Py_INCREF(&Model_Type);
  PyModule_AddObject(mod, "Model", (PyObject*) &Model_Type);

  /* return success. */
  return 0;
}

