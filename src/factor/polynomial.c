
/* include the vfl header. */
#include <vfl/vfl.h>

/* Polynomial: structure for holding polynomial factors.
 */
typedef struct {
  /* factor superclass. */
  Factor super;

  /* sublcass struct members */
}
Polynomial;

/* define documentation strings: */

PyDoc_STRVAR(
  Polynomial_doc,
"Polynomial() -> Polynomial object\n"
"\n");

PyDoc_STRVAR(
  Polynomial_getset_order_doc,
"Polynomial order (read/write)\n"
"\n");

/* Polynomial_mean(): evaluate the polynomial factor mean.
 *  - see factor_mean_fn() for more information.
 */
FACTOR_MEAN (Polynomial) {
  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* compute and return the expectation. */
  return pow(xd, i);
}

/* Polynomial_var(): evaluate the polynomial factor variance.
 *  - see factor_var_fn() for more information.
 */
FACTOR_VAR (Polynomial) {
  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* compute and return the expectation. */
  return pow(xd, i) * pow(xd, j);
}

/* Polynomial_cov(): evaluate the polynomial factor covariance.
 *  - see factor_cov_fn() for more information.
 */
FACTOR_COV (Polynomial) {
  /* get the input values along the factor dimension. */
  double xd1 = vector_get(x1, f->d);
  double xd2 = vector_get(x2, f->d);

  /* initialize the computation. */
  double cov = 0.0;

  /* loop over the powers of the first input. */
  double xi = 1.0;
  for (size_t i = 0; i < f->K; i++) {
    /* loop over powers of the second input. */
    double xj = 1.0;
    for (size_t j = 0; j < f->K; j++) {
      /* include the contribution and update
       * the power of the second input.
       */
      cov += xi * xj;
      xj *= xd2;
    }

    /* update the power of the first input. */
    xi *= xd1;
  }

  /* return the computed result. */
  return cov;
}

/* --- */

/* Polynomial_new(): allocate a new polynomial factor.
 *  - see PyTypeObject.tp_new for details.
 */
VFL_TYPE_NEW (Polynomial) {
  /* allocate a new polynomial factor. */
  Polynomial *self = (Polynomial*) type->tp_alloc(type, 0);
  Factor_reset((Factor*) self);
  if (!self)
    return NULL;

  /* initialize the function pointers. */
  Factor *f = (Factor*) self;
  f->eval      = Polynomial_mean;
  f->mean      = Polynomial_mean;
  f->var       = Polynomial_var;
  f->cov       = Polynomial_cov;

  /* resize to the default size. */
  if (!factor_resize(f, 1, 0, 1)) {
    Py_DECREF(f);
    return NULL;
  }

  /* return the new object. */
  return (PyObject*) self;
}

/* Polynomial_get_order(): method for getting polynomial orders.
 */
static PyObject*
Polynomial_get_order (Factor *f) {
  /* return the order as an integer. */
  return PyLong_FromSize_t(f->K - 1);
}

/* Polynomial_set_order(): method for setting polynomial orders.
 */
static int
Polynomial_set_order (Factor *f, PyObject *value, void *closure) {
  /* get the new value. */
  const size_t v = PyLong_AsSize_t(value);
  if (PyErr_Occurred())
    return -1;

  /* attempt to resize the factor. */
  if (!factor_resize(f, f->D, f->P, v + 1)) {
    PyErr_SetString(PyExc_RuntimeError, "failed to set polynomial order");
    return -1;
  }

  /* return success. */
  return 0;
}

/* Polynomial_getset: property definition structure for polynomial factors.
 */
static PyGetSetDef Polynomial_getset[] = {
  { "order",
    (getter) Polynomial_get_order,
    (setter) Polynomial_set_order,
    Polynomial_getset_order_doc,
    NULL
  },
  { NULL }
};

/* Polynomial_methods: method definition structure for polynomial factors.
 */
static PyMethodDef Polynomial_methods[] = {
  { NULL }
};

/* Polynomial_Type, Polynomial_Type_init() */
VFL_TYPE (Polynomial, Factor, factor)

