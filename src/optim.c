
/* include the vfl header. */
#include <vfl/vfl.h>

/* define documentation strings: */

PyDoc_STRVAR(
  Optim_doc,
"Optim() -> Optim object\n"
"\n");

/* optim_new(): allocation method for optimizers.
 */
static PyObject*
optim_new (PyTypeObject *type, PyObject *args, PyObject *kwargs) {
  /* FIXME: implement optim_new() */ return NULL;
}

/* optim_dealloc(): deallocation method for optimizers.
 */
static void
optim_dealloc (Optim *self) {
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

/* optim_repr(): representation function for optimizers.
 */
static PyObject*
optim_repr (Optim *self) {
  /* build and return the representation string. */
  return PyUnicode_FromFormat("<%s at 0x%x>", Py_TYPE(self)->tp_name,
                              (long) self);
}

/* Optim_getset: property definition structure for optimizers.
 */
static PyGetSetDef Optim_getset[] = {
  { NULL }
};

/* Optim_methods: method definition structure for optimizers.
 */
static PyMethodDef Optim_methods[] = {
  { NULL }
};

/* Optim_Type: type definition structure for optimizers.
 */
PyTypeObject Optim_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "vfl.Optim",                                   /* tp_name           */
  sizeof(Optim),                                 /* tp_basicsize      */
  0,                                             /* tp_itemsize       */
  (destructor) optim_dealloc,                    /* tp_dealloc        */
  0,                                             /* tp_print          */
  0,                                             /* tp_getattr        */
  0,                                             /* tp_setattr        */
  0,                                             /* tp_reserved       */
  (reprfunc) optim_repr,                         /* tp_repr           */
  0,                                             /* tp_as_number      */
  0,                                             /* tp_as_sequence    */
  0,                                             /* tp_as_mapping     */
  0,                                             /* tp_hash           */
  0,                                             /* tp_call           */
  (reprfunc) optim_repr,                         /* tp_str            */
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
  optim_new                                      /* tp_new            */
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

