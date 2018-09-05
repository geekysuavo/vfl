
/* include the vfl header. */
#include <vfl/vfl.h>

/* define documentation strings: */

PyDoc_STRVAR(
  Optim_doc,
"Optim() -> Optim object\n"
"\n");

PyDoc_STRVAR(
  Optim_getset_bound_doc,
"Value of the variational lower bound (read-only)\n"
"\n");

PyDoc_STRVAR(
  Optim_getset_model_doc,
"Associated variational model (read/write)\n"
"\n");

PyDoc_STRVAR(
  Optim_getset_maxiters_doc,
"Maximum number of iterations (read/write)\n"
"\n");

PyDoc_STRVAR(
  Optim_getset_maxsteps_doc,
"Maximum number of line search steps per iteration (read/write)\n"
"\n");

PyDoc_STRVAR(
  Optim_getset_l0_doc,
"Initial line search Lipschitz constant (read/write)\n"
"\n");

PyDoc_STRVAR(
  Optim_getset_dl_doc,
"Line search Lipschitz constant multiplier (read/write)\n"
"\n");

PyDoc_STRVAR(
  Optim_getset_logiters_doc,
"Logging frequency (read/write)\n"
"\n");

PyDoc_STRVAR(
  Optim_getset_logparms_doc,
"Parameter logging flag (read/write)\n"
"\n");

PyDoc_STRVAR(
  Optim_getset_logfile_doc,
"Filename for writing logging data (write-only)\n"
"\n");

PyDoc_STRVAR(
  Optim_method_execute_doc,
"Execute a round of free-running optimization.\n"
"\n");

PyDoc_STRVAR(
  Optim_method_iterate_doc,
"Execute a single optimization iteration.\n"
"\n");

/* Optim_get_bound(): method to get the lower bound of an optimizer.
 */
static PyObject*
Optim_get_bound (Optim *self) {
  /* return the lower bound as a float. */
  return PyFloat_FromDouble(self->bound);
}

/* Optim_get_model(): method to get the model of an optimizer.
 */
static PyObject*
Optim_get_model (Optim *self) {
  /* return nothing if the model is null. */
  if (!self->mdl)
    Py_RETURN_NONE;

  /* return a new reference to the model. */
  Py_INCREF(self->mdl);
  return (PyObject*) self->mdl;
}

/* Optim_set_model(): method to set the model of an optimizer.
 */
static int
Optim_set_model (Optim *self, PyObject *value, void *closure) {
  /* check that the value is a model. */
  if (!value || !Model_Check(value)) {
    PyErr_SetString(PyExc_TypeError, "expected Model object");
    return -1;
  }

  /* attempt to set the new model. */
  if (!optim_set_model(self, (Model*) value)) {
    PyErr_SetString(PyExc_RuntimeError, "failed to set model");
    return -1;
  }

  /* return success. */
  return 0;
}

/* Optim_get_maxiters(): method to get the iteration limit of an optimizer.
 */
static PyObject*
Optim_get_maxiters (Optim *self) {
  /* return the iteration limit as an integer. */
  return PyLong_FromSize_t(self->max_iters);
}

/* Optim_set_maxiters(): method to set the iteration limit of an optimizer.
 */
static int
Optim_set_maxiters (Optim *self, PyObject *value, void *closure) {
  /* get the new value. */
  const size_t v = PyLong_AsSize_t(value);
  if (PyErr_Occurred())
    return -1;

  /* set the new value. */
  if (!optim_set_max_iters(self, v)) {
    PyErr_SetString(PyExc_ValueError, "expected positive integer");
    return -1;
  }

  /* return success. */
  return 0;
}

/* Optim_get_maxsteps(): method to get the step limit of an optimizer.
 */
static PyObject*
Optim_get_maxsteps (Optim *self) {
  /* return the step limit as an integer. */
  return PyLong_FromSize_t(self->max_steps);
}

/* Optim_set_maxsteps(): method to set the step limit of an optimizer.
 */
static int
Optim_set_maxsteps (Optim *self, PyObject *value, void *closure) {
  /* get the new value. */
  const size_t v = PyLong_AsSize_t(value);
  if (PyErr_Occurred())
    return -1;

  /* set the new value. */
  if (!optim_set_max_steps(self, v)) {
    PyErr_SetString(PyExc_ValueError, "expected positive integer");
    return -1;
  }

  /* return success. */
  return 0;
}

/* Optim_get_l0(): method to get the initial lipschitz constant
 * of an optimizer.
 */
static PyObject*
Optim_get_l0 (Optim *self) {
  /* return the value as a float. */
  return PyFloat_FromDouble(self->l0);
}

/* Optim_set_l0(): method to set the initial lipschitz constant
 * of an optimizer.
 */
static int
Optim_set_l0 (Optim *self, PyObject *value, void *closure) {
  /* get the new value. */
  const double v = PyFloat_AsDouble(value);
  if (PyErr_Occurred())
    return -1;

  /* set the new value. */
  if (!optim_set_lipschitz_init(self, v)) {
    PyErr_SetString(PyExc_ValueError, "expected positive float");
    return -1;
  }

  /* return success. */
  return 0;
}

/* Optim_get_dl(): method to get the lipschitz adjustment factor
 * of an optimizer.
 */
static PyObject*
Optim_get_dl (Optim *self) {
  /* return the value as a float. */
  return PyFloat_FromDouble(self->dl);
}

/* Optim_set_dl(): method to set the lipschitz adjustment factor
 * of an optimizer.
 */
static int
Optim_set_dl (Optim *self, PyObject *value, void *closure) {
  /* get the new value. */
  const double v = PyFloat_AsDouble(value);
  if (PyErr_Occurred())
    return -1;

  /* set the new value. */
  if (!optim_set_lipschitz_step(self, v)) {
    PyErr_SetString(PyExc_ValueError, "expected positive float");
    return -1;
  }

  /* return success. */
  return 0;
}

/* Optim_get_logiters(): method to get the logging frequency
 * of an optimizer.
 */
static PyObject*
Optim_get_logiters (Optim *self) {
  /* return the iteration frequency as an integer. */
  return PyLong_FromSize_t(self->log_iters);
}

/* Optim_set_logiters(): method to set the logging frequency
 * of an optimizer.
 */
static int
Optim_set_logiters (Optim *self, PyObject *value, void *closure) {
  /* get the new value. */
  const size_t v = PyLong_AsSize_t(value);
  if (PyErr_Occurred())
    return -1;

  /* set the new value. */
  if (!optim_set_log_iters(self, v)) {
    PyErr_SetString(PyExc_ValueError, "expected positive integer");
    return -1;
  }

  /* return success. */
  return 0;
}

/* Optim_get_logparms(): method to get the parameter logging flag
 * of an optimizer.
 */
static PyObject*
Optim_get_logparms (Optim *self) {
  /* return the parameter logging flag as a boolean. */
  return PyBool_FromLong(self->log_parms);
}

/* Optim_set_logparms(): method to set the parameter logging flag
 * of an optimizer.
 */
static int
Optim_set_logparms (Optim *self, PyObject *value, void *closure) {
  /* check that the value is a boolean. */
  if (!PyBool_Check(value)) {
    PyErr_SetString(PyExc_TypeError, "'log_parms' expects bool");
    return -1;
  }

  /* set the value and return success. */
  optim_set_log_parms(self, (int) PyLong_AsLong(value));
  return 0;
}

/* Optim_set_logfile(): method to set the log file of an optimizer.
 */
static int
Optim_set_logfile (Optim *self, PyObject *value, void *closure) {
  /* check that the value is a unicode type. */
  if (!PyUnicode_Check(value)) {
    PyErr_SetString(PyExc_TypeError, "'log_file' expects str");
    return -1;
  }

  /* get the filename as a c string. */
  const char *fname = PyUnicode_AsUTF8AndSize(value, NULL);
  if (!fname)
    return -1;

  /* attempt to open the log file. */
  if (!optim_set_log_file(self, fname)) {
    PyErr_SetNone(PyExc_IOError);
    return -1;
  }

  /* return success. */
  return 0;
}

/* --- */

/* Optim_method_execute(): execute a round of optimization.
 */
static PyObject*
Optim_method_execute (Optim *self, PyObject *args, PyObject *kwargs) {
  /* execute and return nothing. */
  optim_execute(self);
  Py_RETURN_NONE;
}

/* Optim_method_iterate(): execute a single iteration.
 */
static PyObject*
Optim_method_iterate (Optim *self, PyObject *args, PyObject *kwargs) {
  /* iterate and return the result. */
  return PyBool_FromLong(optim_iterate(self));
}

/* --- */

/* Optim_new(): allocation method for optimizers.
 */
static PyObject*
Optim_new (PyTypeObject *type, PyObject *args, PyObject *kwargs) {
  /* allocate a new optimizer. */
  Optim *self = (Optim*) type->tp_alloc(type, 0);
  Optim_reset(self);
  if (!self)
    return NULL;

  /* if the optimizer has an init function assigned, call it. */
  if (self->init && !self->init(self))
    return NULL;

  /* return the new object. */
  return (PyObject*) self;
}

/* Optim_dealloc(): deallocation method for optimizers.
 */
static void
Optim_dealloc (Optim *self) {
  /* if the optimizer has a free function assigned, call it. */
  if (self->free)
    self->free(self);

  /* release the reference to the associated model. */
  Py_XDECREF(self->mdl);

  /* close any open file handles. */
  if (self->log_fh)
    fclose(self->log_fh);

  /* free the iteration vectors. */
  vector_free(self->xa);
  vector_free(self->xb);
  vector_free(self->x);
  vector_free(self->g);

  /* free the temporaries. */
  matrix_free(self->Fs);

  /* release the object memory. */
  Py_TYPE(self)->tp_free((PyObject*) self);
}

/* Optim_repr(): representation function for optimizers.
 */
static PyObject*
Optim_repr (Optim *self) {
  /* build and return the representation string. */
  return PyUnicode_FromFormat("<%s at 0x%x>", Py_TYPE(self)->tp_name,
                              (long) self);
}

/* Optim_getset: property definition structure for optimizers.
 */
static PyGetSetDef Optim_getset[] = {
  { "bound",
    (getter) Optim_get_bound,
    NULL,
    Optim_getset_bound_doc,
    NULL
  },
  { "model",
    (getter) Optim_get_model,
    (setter) Optim_set_model,
    Optim_getset_model_doc,
    NULL
  },
  { "max_iters",
    (getter) Optim_get_maxiters,
    (setter) Optim_set_maxiters,
    Optim_getset_maxiters_doc,
    NULL
  },
  { "max_steps",
    (getter) Optim_get_maxsteps,
    (setter) Optim_set_maxsteps,
    Optim_getset_maxsteps_doc,
    NULL
  },
  { "lipschitz_init",
    (getter) Optim_get_l0,
    (setter) Optim_set_l0,
    Optim_getset_l0_doc,
    NULL
  },
  { "lipschitz_step",
    (getter) Optim_get_dl,
    (setter) Optim_set_dl,
    Optim_getset_dl_doc,
    NULL
  },
  { "log_iters",
    (getter) Optim_get_logiters,
    (setter) Optim_set_logiters,
    Optim_getset_logiters_doc,
    NULL
  },
  { "log_parms",
    (getter) Optim_get_logparms,
    (setter) Optim_set_logparms,
    Optim_getset_logparms_doc,
    NULL
  },
  { "log_file",
    NULL,
    (setter) Optim_set_logfile,
    Optim_getset_logfile_doc,
    NULL
  },
  { NULL }
};

/* Optim_methods: method definition structure for optimizers.
 */
static PyMethodDef Optim_methods[] = {
  { "execute",
    (PyCFunction) Optim_method_execute,
    METH_VARARGS,
    Optim_method_execute_doc
  },
  { "iterate",
    (PyCFunction) Optim_method_iterate,
    METH_VARARGS,
    Optim_method_iterate_doc
  },
  { NULL }
};

/* Optim_Type: type definition structure for optimizers.
 */
PyTypeObject Optim_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "vfl.Optim",                                   /* tp_name           */
  sizeof(Optim),                                 /* tp_basicsize      */
  0,                                             /* tp_itemsize       */
  (destructor) Optim_dealloc,                    /* tp_dealloc        */
  0,                                             /* tp_print          */
  0,                                             /* tp_getattr        */
  0,                                             /* tp_setattr        */
  0,                                             /* tp_reserved       */
  (reprfunc) Optim_repr,                         /* tp_repr           */
  0,                                             /* tp_as_number      */
  0,                                             /* tp_as_sequence    */
  0,                                             /* tp_as_mapping     */
  0,                                             /* tp_hash           */
  0,                                             /* tp_call           */
  (reprfunc) Optim_repr,                         /* tp_str            */
  0,                                             /* tp_getattro       */
  0,                                             /* tp_setattro       */
  0,                                             /* tp_as_buffer      */
  Py_TPFLAGS_DEFAULT |
  Py_TPFLAGS_BASETYPE,                           /* tp_flags          */
  Optim_doc,                                     /* tp_doc            */
  0,                                             /* tp_traverse       */
  0,                                             /* tp_clear          */
  0,                                             /* tp_richcompare    */
  0,                                             /* tp_weaklistoffset */
  0,                                             /* tp_iter           */
  0,                                             /* tp_iternext       */
  Optim_methods,                                 /* tp_methods        */
  0,                                             /* tp_members        */
  Optim_getset,                                  /* tp_getset         */
  0,                                             /* tp_base           */
  0,                                             /* tp_dict           */
  0,                                             /* tp_descr_get      */
  0,                                             /* tp_descr_set      */
  0,                                             /* tp_dictoffset     */
  vfl_base_init,                                 /* tp_init           */
  0,                                             /* tp_alloc          */
  Optim_new                                      /* tp_new            */
};

/* Optim_Type_init(): type initialization function for optimizers.
 */
int
Optim_Type_init (PyObject *mod) {
  /* finalize the type object. */
  if (PyType_Ready(&Optim_Type) < 0)
    return -1;

  /* take a reference to the type and add it to the module. */
  Py_INCREF(&Optim_Type);
  PyModule_AddObject(mod, "Optim", (PyObject*) &Optim_Type);

  /* return success. */
  return 0;
}

