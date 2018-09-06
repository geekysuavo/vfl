
/* include the vfl header. */
#include <vfl/vfl.h>

/* define the parameter indices. */
#define P_TAU  0

/* FixedImpulse: structure for holding fixed impulse factors.
 */
typedef struct {
  /* factor superclass. */
  Factor super;

  /* subclass struct members:
   *  @mu: fixed location parameter.
   */
  double mu;
}
FixedImpulse;

/* define documentation strings: */

PyDoc_STRVAR(
  FixedImpulse_doc,
"FixedImpulse() -> FixedImpulse object\n"
"\n");

PyDoc_STRVAR(
  FixedImpulse_getset_mu_doc,
"Fixed impulse location (read/write)\n"
"\n");

PyDoc_STRVAR(
  FixedImpulse_getset_tau_doc,
"Impulse location precision (read/write)\n"
"\n");

/* FixedImpulse_eval(): evaluate the fixed impulse factor at its mode.
 *  - see factor_mean_fn() for more information.
 */
FACTOR_EVAL (FixedImpulse) {
  /* get the location parameter. */
  FixedImpulse *fx = (FixedImpulse*) f;
  const double mu = fx->mu;

  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* return the delta function value. */
  return (xd == mu ? 1.0 : 0.0);
}

/* FixedImpulse_mean(): evaluate the fixed impulse factor mean.
 *  - see factor_mean_fn() for more information.
 */
FACTOR_MEAN (FixedImpulse) {
  /* get the location parameter. */
  FixedImpulse *fx = (FixedImpulse*) f;
  const double mu = fx->mu;

  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* get the factor parameter. */
  const double tau = vector_get(f->par, P_TAU);

  /* compute the normalization and shift. */
  const double u = xd - mu;

  /* compute and return the expectation. */
  return exp(-0.5 * tau * u * u);
}

/* FixedImpulse_var(): evaluate the fixed impulse factor variance.
 *  - see factor_var_fn() for more information.
 */
FACTOR_VAR (FixedImpulse) {
  /* call the mean function. */
  return FixedImpulse_mean(f, x, p, i);
}

/* FixedImpulse_diff_mean(): evaluate the fixed impulse factor
 * mean gradient.
 *  - see factor_diff_mean_fn() for more information.
 */
FACTOR_DIFF_MEAN (FixedImpulse) {
  /* get the location parameter. */
  FixedImpulse *fx = (FixedImpulse*) f;
  const double mu = fx->mu;

  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* compute the mean value and the shift. */
  const double Gx = FixedImpulse_mean(f, x, p, i);
  const double u = xd - mu;

  /* compute and store the partial derivative. */
  const double dtau = -0.5 * (u * u) * Gx;
  vector_set(df, P_TAU, dtau);
}

/* FixedImpulse_diff_var(): evaluate the fixed impulse factor
 * variance gradient.
 *  - see factor_diff_var_fn() for more information.
 */
FACTOR_DIFF_VAR (FixedImpulse) {
  /* call the mean gradient function. */
  FixedImpulse_diff_mean(f, x, p, i, df);
}

/* FixedImpulse_div(): evaluate the fixed impulse factor divergence.
 *  - see factor_div_fn() for more information.
 */
FACTOR_DIV (FixedImpulse) {
  /* get the first factor parameters. */
  const double tau = vector_get(f->par, P_TAU);
  FixedImpulse *fx = (FixedImpulse*) f;
  const double mu = fx->mu;

  /* get the second factor parameters. */
  const double tau2 = vector_get(f2->par, P_TAU);
  FixedImpulse *f2x = (FixedImpulse*) f2;
  const double mu2 = f2x->mu;

  /* compute the divergence. */
  return 0.5 * tau2 * (mu * mu + 1.0 / tau - 2.0 * mu * mu2 + mu2 * mu2)
       - 0.5 * log(tau2 / tau) - 0.5;
}

/* FixedImpulse_set(): store a parameter into a fixed impulse factor.
 *  - see factor_set_fn() for more information.
 */
FACTOR_SET (FixedImpulse) {
  /* determine which parameter is being assigned. */
  switch (i) {
    /* location precision: in (0, inf) */
    case P_TAU:;
      const double tau = value;
      if (tau <= 0.0)
        return 0;

      vector_set(f->par, i, tau);
      matrix_set(f->inf, P_TAU, P_TAU, 0.75 / (tau * tau));

      return 1;
  }

  /* invalid parameter index. */
  return 0;
}

/* FixedImpulse_copy(): copy extra information between fixed impulse
 * factors.
 *  - see factor_copy_fn() for more information.
 */
FACTOR_COPY (FixedImpulse) {
  /* get the extended structure pointers. */
  FixedImpulse *fdupx = (FixedImpulse*) fdup;
  FixedImpulse *fx = (FixedImpulse*) f;

  /* copy the location parameter. */
  fdupx->mu = fx->mu;

  /* return success. */
  return 1;
}

/* --- */

/* FixedImpulse_new(): allocate a new fixed impulse factor.
 *  - see PyTypeObject.tp_new for details.
 */
VFL_TYPE_NEW (FixedImpulse) {
  /* allocate a new fixed impulse factor. */
  FixedImpulse *self = (FixedImpulse*) type->tp_alloc(type, 0);
  Factor_reset((Factor*) self);
  if (!self)
    return NULL;

  /* initialize the function pointers. */
  Factor *f = (Factor*) self;
  f->eval      = FixedImpulse_eval;
  f->mean      = FixedImpulse_mean;
  f->var       = FixedImpulse_var;
  f->diff_mean = FixedImpulse_diff_mean;
  f->diff_var  = FixedImpulse_diff_var;
  f->div       = FixedImpulse_div;
  f->set       = FixedImpulse_set;

  /* resize to the default size. */
  if (!factor_resize(f, 1, 1, 1)) {
    Py_DECREF(f);
    return NULL;
  }

  /* set the default parameter values. */
  self->mu = 0.0;
  factor_set(f, P_TAU, 1.0);

  /* return the new object. */
  return (PyObject*) self;
}

/* FixedImpulse_get_mu(): method for getting fixed impulse locations.
 */
static PyObject*
FixedImpulse_get_mu (FixedImpulse *fx) {
  /* return the fixed location parameter as a float. */
  return PyFloat_FromDouble(fx->mu);
}

/* FixedImpulse_set_mu(): method for setting fixed impulse locations.
 */
static int
FixedImpulse_set_mu (FixedImpulse *fx, PyObject *value, void *closure) {
  /* get the new value. */
  const double v = PyFloat_AsDouble(value);
  if (PyErr_Occurred())
    return -1;

  /* set the new value and return success. */
  fx->mu = v;
  return 0;
}

/* fixed impulse factor properties. */
FACTOR_PROP_GETSET (FixedImpulse, tau, P_TAU)

/* FixedImpulse_getset: property definition structure for
 * fixed impulse factors.
 */
static PyGetSetDef FixedImpulse_getset[] = {
  { "mu",
    (getter) FixedImpulse_get_mu,
    (setter) FixedImpulse_set_mu,
    FixedImpulse_getset_mu_doc,
    NULL
  },
  FACTOR_PROP (FixedImpulse, tau),
  { NULL }
};

/* FixedImpulse_methods: method definition structure for
 * fixed impulse factors.
 */
static PyMethodDef FixedImpulse_methods[] = {
  { NULL }
};

/* FixedImpulse_Type, FixedImpulse_Type_init() */
VFL_TYPE (FixedImpulse, Factor, factor)

