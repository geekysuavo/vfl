
#include <vfl/vfl.h>

/* declare sub-module initialization functions: */

PyMODINIT_FUNC PyInit_factor (void);
PyMODINIT_FUNC PyInit_model (void);
PyMODINIT_FUNC PyInit_optim (void);

/* declare type initialization functions: */

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

  /* FIXME: intialize the core module types. */
  if (Datum_Type_init(vfl) < 0 ||
      Data_Type_init(vfl) < 0)
    return NULL;

  /* FIXME: initialize the factor sub-module. */

  /* FIXME: initialize the model sub-module. */

  /* FIXME: initialize the optim sub-module. */

  /* return the new module. */
  return vfl;
}

