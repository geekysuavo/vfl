
/* include the vfl header. */
#include <vfl/vfl.h>

/* declare type initialization functions: */

int VFC_Type_init (PyObject *mod);
int VFR_Type_init (PyObject *mod);
int TauVFR_Type_init (PyObject *mod);

/* define documentation strings: */

PyDoc_STRVAR(
  model_module_doc,
"VFL -- Models.\n"
"\n");

/* model_module: module definition structure for vfl model types.
 */
static PyModuleDef model_module = {
  PyModuleDef_HEAD_INIT,
  "model",                                       /* m_name     */
  model_module_doc,                              /* m_doc      */
  -1,                                            /* m_size     */
  NULL,                                          /* m_methods  */
  NULL,                                          /* m_slots    */
  NULL,                                          /* m_traverse */
  NULL,                                          /* m_clear    */
  NULL                                           /* m_free     */
};

/* PyInit_model: vfl model module initialization function.
 */
PyMODINIT_FUNC
PyInit_model (void) {
  /* create a new model module. */
  PyObject *model = PyModule_Create(&model_module);
  if (!model)
    return NULL;

  /* initialize the model types. */
  if (VFC_Type_init(model) < 0 ||
      VFR_Type_init(model) < 0 ||
      TauVFR_Type_init(model))
    return NULL;

  /* return the new module. */
  return model;
}

