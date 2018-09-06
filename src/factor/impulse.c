
/* include the vfl header. */
#include <vfl/vfl.h>

/* define the parameter indices. */
#define P_MU   0
#define P_TAU  1

/* Impulse: structure for holding impulse factors.
 */
typedef struct {
  /* factor superclass. */
  Factor super;

  /* subclass struct members. */
}
Impulse;

/* define documentation strings: */

PyDoc_STRVAR(
  Impulse_doc,
"Impulse() -> Impulse object\n"
"\n");

PyDoc_STRVAR(
  Impulse_getset_mu_doc,
"Impulse location mean (read/write)\n"
"\n");

PyDoc_STRVAR(
  Impulse_getset_tau_doc,
"Impulse location precision (read/write)\n"
"\n");

/* Impulse_eval(): evaluate the impulse factor at its mode.
 *  - see factor_mean_fn() for more information.
 */
FACTOR_EVAL (Impulse) {
  /* get the input value and the factor mean. */
  const double xd = vector_get(x, f->d);
  const double mu = vector_get(f->par, P_MU);

  /* return the delta function value. */
  return (xd == mu ? 1.0 : 0.0);
}

/* Impulse_mean(): evaluate the impulse factor mean.
 *  - see factor_mean_fn() for more information.
 */
FACTOR_MEAN (Impulse) {
  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* get the factor parameters. */
  const double mu = vector_get(f->par, P_MU);
  const double tau = vector_get(f->par, P_TAU);

  /* compute the normalization and shift. */
  const double u = xd - mu;

  /* compute and return the expectation. */
  return exp(-0.5 * tau * u * u);
}

/* Impulse_var(): evaluate the impulse factor variance.
 *  - see factor_var_fn() for more information.
 */
FACTOR_VAR (Impulse) {
  /* call the mean function. */
  return Impulse_mean(f, x, p, i);
}

/* Impulse_diff_mean(): evaluate the impulse factor mean gradient.
 *  - see factor_diff_mean_fn() for more information.
 */
FACTOR_DIFF_MEAN (Impulse) {
  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* get the factor parameters. */
  const double mu = vector_get(f->par, P_MU);
  const double tau = vector_get(f->par, P_TAU);

  /* compute the mean value and the shift. */
  const double Gx = Impulse_mean(f, x, p, i);
  const double u = xd - mu;

  /* compute the partial derivatives. */
  const double dmu = Gx * tau * u;
  const double dtau = -0.5 * (u * u) * Gx;

  /* store the partial derivatives. */
  vector_set(df, P_MU, dmu);
  vector_set(df, P_TAU, dtau);
}

/* Impulse_diff_var(): evaluate the impulse factor variance gradient.
 *  - see factor_diff_var_fn() for more information.
 */
FACTOR_DIFF_VAR (Impulse) {
  /* call the mean gradient function. */
  Impulse_diff_mean(f, x, p, i, df);
}

/* Impulse_div(): evaluate the impulse factor divergence.
 *  - see factor_div_fn() for more information.
 */
FACTOR_DIV (Impulse) {
  /* get the first factor parameters. */
  const double mu = vector_get(f->par, P_MU);
  const double tau = vector_get(f->par, P_TAU);

  /* get the second factor parameters. */
  const double mu2 = vector_get(f2->par, P_MU);
  const double tau2 = vector_get(f2->par, P_TAU);

  /* compute the divergence. */
  return 0.5 * tau2 * (mu * mu + 1.0 / tau - 2.0 * mu * mu2 + mu2 * mu2)
       - 0.5 * log(tau2 / tau) - 0.5;
}

/* Impulse_set(): store a parameter into a impulse factor.
 *  - see factor_set_fn() for more information.
 */
FACTOR_SET (Impulse) {
  /* determine which parameter is being assigned. */
  switch (i) {
    /* location mean: in (-inf, inf) */
    case P_MU:;
      const double mu = value;
      vector_set(f->par, i, mu);

      return 1;

    /* location precision: in (0, inf) */
    case P_TAU:;
      const double tau = value;
      if (tau <= 0.0)
        return 0;

      vector_set(f->par, i, tau);

      matrix_set(f->inf, P_MU, P_MU, tau);
      matrix_set(f->inf, P_TAU, P_TAU, 0.75 / (tau * tau));

      return 1;
  }

  /* invalid parameter index. */
  return 0;
}

/* --- */

/* Impulse_new(): allocate a new impulse factor.
 *  - see PyTypeObject.tp_new for details.
 */
VFL_TYPE_NEW (Impulse) {
  /* allocate a new impulse factor. */
  Impulse *self = (Impulse*) type->tp_alloc(type, 0);
  Factor_reset((Factor*) self);
  if (!self)
    return NULL;

  /* initialize the function pointers. */
  Factor* f = (Factor*) self;
  f->eval      = Impulse_eval;
  f->mean      = Impulse_mean;
  f->var       = Impulse_var;
  f->diff_mean = Impulse_diff_mean;
  f->diff_var  = Impulse_diff_var;
  f->div       = Impulse_div;
  f->set       = Impulse_set;

  /* resize to the default size. */
  if (!factor_resize(f, 1, 2, 1)) {
    Py_DECREF(f);
    return NULL;
  }

  /* set the default parameter values. */
  factor_set(f, P_MU, 0.0);
  factor_set(f, P_TAU, 1.0);

  /* return the new object. */
  return (PyObject*) self;
}

/* impulse factor properties. */
FACTOR_PROP_GETSET (Impulse, mu,  P_MU)
FACTOR_PROP_GETSET (Impulse, tau, P_TAU)

/* Impulse_getset: property definition structure for impulse factors.
 */
static PyGetSetDef Impulse_getset[] = {
  FACTOR_PROP (Impulse, mu),
  FACTOR_PROP (Impulse, tau),
  { NULL }
};

/* Impulse_methods: method definition structure for impulse factors.
 */
static PyMethodDef Impulse_methods[] = {
  { NULL }
};

/* Impulse_Type, Impulse_Type_init() */
VFL_TYPE (Impulse, Factor, factor)

