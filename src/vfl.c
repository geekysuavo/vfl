
#include <vfl/vfl.h>

/* declare sub-module initialization functions: */

PyMODINIT_FUNC PyInit_factor (void);
PyMODINIT_FUNC PyInit_model (void);
PyMODINIT_FUNC PyInit_optim (void);

/* declare type initialization functions: */

int Search_Type_init (PyObject *mod);
int Factor_Type_init (PyObject *mod);
int Model_Type_init (PyObject *mod);
int Optim_Type_init (PyObject *mod);
int Datum_Type_init (PyObject *mod);
int Data_Type_init (PyObject* mod);

/* define documentation strings: */

PyDoc_STRVAR(
  vfl_module_doc,
"Variational Feature Learning.\n"
"\n"
"This module defines general classes required for VFL,\n"
"including data, factors, models, and optimizers.\n"
);

/* vfl_module: module definition structure for vfl core types.
 */
static PyModuleDef vfl_module = {
  PyModuleDef_HEAD_INIT,
  "vfl",                                         /* m_name     */
  vfl_module_doc,                                /* m_doc      */
  -1,                                            /* m_size     */
  NULL,                                          /* m_methods  */
  NULL,                                          /* m_slots    */
  NULL,                                          /* m_traverse */
  NULL,                                          /* m_clear    */
  NULL                                           /* m_free     */
};

/* PyInit_vfl: vfl module initialization function.
 */
PyMODINIT_FUNC
PyInit_vfl (void) {
  /* create a new core module. */
  PyObject *vfl = PyModule_Create(&vfl_module);
  if (!vfl)
    return NULL;

  /* intialize the core module types. */
  if (Search_Type_init(vfl) < 0 ||
      Factor_Type_init(vfl) < 0 ||
      Model_Type_init(vfl) < 0 ||
      Optim_Type_init(vfl) < 0 ||
      Datum_Type_init(vfl) < 0 ||
      Data_Type_init(vfl) < 0)
    return NULL;

  /* initialize the factor sub-module. */
  PyObject *factor = PyInit_factor();
  if (factor) {
    Py_INCREF(factor);
    PyModule_AddObject(vfl, "factor", factor);
  }
  else
    return NULL;

  /* initialize the model sub-module. */
  PyObject *model = PyInit_model();
  if (model) {
    Py_INCREF(model);
    PyModule_AddObject(vfl, "model", model);
  }
  else
    return NULL;

  /* initialize the optimizer sub-module. */
  PyObject *optim = PyInit_optim();
  if (optim) {
    Py_INCREF(optim);
    PyModule_AddObject(vfl, "optim", optim);
  }
  else
    return NULL;

  /* return the new module. */
  return vfl;
}

/* vfl_base_init(): initialization method for factors, models, and
 * optimizers in vfl.
 */
int
vfl_base_init (PyObject *self, PyObject *args, PyObject *kwargs) {
  /* return if no keyword arguments were given. */
  if (!kwargs || !PyDict_Check(kwargs))
    return 0;

  /* declare variables for dictionary traversal. */
  PyObject *key, *val;
  Py_ssize_t i = 0;

  /* treat each dictionary key-value pair as an attribute to set. */
  while (PyDict_Next(kwargs, &i, &key, &val)) {
    if (PyObject_SetAttr((PyObject*) self, key, val) < 0)
      return -1;
  }

  /* return success. */
  return 0;
}

