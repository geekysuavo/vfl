
/* include the vfl header. */
#include <vfl/vfl.h>

/* declare type initialization functions: */

int Cosine_Type_init (PyObject *mod);
int Decay_Type_init (PyObject *mod);
int FixedImpulse_Type_init (PyObject *mod);
int Impulse_Type_init (PyObject *mod);
int Polynomial_Type_init (PyObject *mod);
int Product_Type_init (PyObject *mod);

/* define documentation strings: */

PyDoc_STRVAR(
  factor_module_doc,
"VFL -- Factors.\n"
"\n");

/* factor_module: module definition structure for vfl factor types.
 */
static PyModuleDef factor_module = {
  PyModuleDef_HEAD_INIT,
  "factor",                                      /* m_name     */
  factor_module_doc,                             /* m_doc      */
  -1,                                            /* m_size     */
  NULL,                                          /* m_methods  */
  NULL,                                          /* m_slots    */
  NULL,                                          /* m_traverse */
  NULL,                                          /* m_clear    */
  NULL                                           /* m_free     */
};

/* PyInit_factor: vfl factor module initialization function.
 */
PyMODINIT_FUNC
PyInit_factor (void) {
  /* create a new factor module. */
  PyObject *factor = PyModule_Create(&factor_module);
  if (!factor)
    return NULL;

  /* initialize the factor types. */
  if (Cosine_Type_init(factor) < 0 ||
      Decay_Type_init(factor) < 0 ||
      FixedImpulse_Type_init(factor) < 0 ||
      Impulse_Type_init(factor) < 0 ||
      Polynomial_Type_init(factor) < 0 ||
      Product_Type_init(factor) < 0)
    return NULL;

  /* return the new module. */
  return factor;
}

