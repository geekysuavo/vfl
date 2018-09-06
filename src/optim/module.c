
/* include the vfl header. */
#include <vfl/vfl.h>

/* declare type initialization functions: */

int FullGradient_Type_init (PyObject *mod);
int MeanField_Type_init (PyObject *mod);

/* define documentation strings: */

PyDoc_STRVAR(
  optim_module_doc,
"VFL -- Optimizers.\n"
"\n");

/* optim_module: module definition structure for vfl optimizer types.
 */
static PyModuleDef optim_module = {
  PyModuleDef_HEAD_INIT,
  "optim",                                       /* m_name     */
  optim_module_doc,                              /* m_doc      */
  -1,                                            /* m_size     */
  NULL,                                          /* m_methods  */
  NULL,                                          /* m_slots    */
  NULL,                                          /* m_traverse */
  NULL,                                          /* m_clear    */
  NULL                                           /* m_free     */
};

/* PyInit_optim: vfl optimizer module initialization function.
 */
PyMODINIT_FUNC
PyInit_optim (void) {
  /* create a new optimizer module. */
  PyObject *optim = PyModule_Create(&optim_module);
  if (!optim)
    return NULL;

  /* initialize the optimizer types. */
  if (FullGradient_Type_init(optim) < 0 ||
      MeanField_Type_init(optim) < 0)
    return NULL;

  /* return the new module. */
  return optim;
}

