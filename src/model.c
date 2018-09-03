
/* include the vfl header. */
#include <vfl/vfl.h>

/* define documentation strings: */

PyDoc_STRVAR(
  Model_doc,
"Model() -> Model object\n"
"\n");

/* model_new(): allocation method for models.
 */
static PyObject*
model_new (PyTypeObject *type, PyObject *args, PyObject *kwargs) {
  /* FIXME: implement model_new() */ return NULL;
}

/* model_dealloc(): deallocation method for models.
 */
static void
model_dealloc (Model *self) {
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

  /* free the factor arrays. */
  free(self->factors);
  free(self->priors);

  /* free the temporary vector. */
  vector_free(self->tmp);

  /* release the object memory. */
  Py_TYPE(self)->tp_free((PyObject*) self);
}

/* model_repr(): representation function for models.
 */
static PyObject*
model_repr (Model *self) {
  /* build and return the representation string. */
  return PyUnicode_FromFormat("<%s at 0x%x>", Py_TYPE(self)->tp_name,
                              (long) self);
}

/* model_call(): evaluation method for models.
 */
static PyObject*
model_call (Model *self, PyObject *args, PyObject *kwargs) {
  /* FIXME: implement model_call() */ return NULL;
}

/* Model_getset: property definition structure for models.
 */
static PyGetSetDef Model_getset[] = {
  { NULL }
};

/* Model_methods: method definition structure for models.
 */
static PyMethodDef Model_methods[] = {
  { NULL }
};

/* Model_Type: type definition structure for models.
 */
PyTypeObject Model_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "vfl.Model",                                   /* tp_name           */
  sizeof(Model),                                 /* tp_basicsize      */
  0,                                             /* tp_itemsize       */
  (destructor) model_dealloc,                    /* tp_dealloc        */
  0,                                             /* tp_print          */
  0,                                             /* tp_getattr        */
  0,                                             /* tp_setattr        */
  0,                                             /* tp_reserved       */
  (reprfunc) model_repr,                         /* tp_repr           */
  0,                                             /* tp_as_number      */
  0,                                             /* tp_as_sequence    */
  0,                                             /* tp_as_mapping     */
  0,                                             /* tp_hash           */
  (ternaryfunc) model_call,                      /* tp_call           */
  (reprfunc) model_repr,                         /* tp_str            */
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
  model_new                                      /* tp_new            */
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

