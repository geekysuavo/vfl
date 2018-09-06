
/* include the vfl header. */
#include <vfl/vfl.h>

/* define the parameter indices. */
#define P_ALPHA  0
#define P_BETA   1

/* Decay: structure for holding decay factors.
 */
typedef struct {
  /* factor superclass. */
  Factor super;

  /* subclass struct members. */
}
Decay;

/* define documentation strings: */

PyDoc_STRVAR(
  Decay_doc,
"Decay() -> Decay object\n"
"\n");

PyDoc_STRVAR(
  Decay_getset_alpha_doc,
"Decay rate, shape parameter (read/write)\n"
"\n");

PyDoc_STRVAR(
  Decay_getset_beta_doc,
"Decay rate, rate parameter (read/write)\n"
"\n");

/* Decay_eval(): evaluate the decay factor at its mode
 *  - see factor_mean_fn() for more information.
 */
FACTOR_EVAL (Decay) {
  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* get the factor parameters. */
  const double alpha = vector_get(f->par, P_ALPHA);
  const double beta = vector_get(f->par, P_BETA);

  /* compute the most probable decay rate. */
  const double rho = (alpha - 1.0) / beta;

  /* evaluate the factor. */
  return exp(-rho * xd);
}

/* Decay_mean(): evaluate the decay factor mean.
 *  - see factor_mean_fn() for more information.
 */
FACTOR_MEAN (Decay) {
  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* get the factor parameters. */
  const double alpha = vector_get(f->par, P_ALPHA);
  const double beta = vector_get(f->par, P_BETA);

  /* compute and return the expectation. */
  return pow(beta / (beta + xd), alpha);
}

/* Decay_var(): evaluate the decay factor variance.
 *  - see factor_var_fn() for more information.
 */
FACTOR_VAR (Decay) {
  /* get twice the input value along the factor dimension. */
  const double xp = 2.0 * vector_get(x, f->d);

  /* get the factor parameters. */
  const double alpha = vector_get(f->par, P_ALPHA);
  const double beta = vector_get(f->par, P_BETA);

  /* compute and return the expectation. */
  return pow(beta / (beta + xp), alpha);
}

/* Decay_cov(): evaluate the decay factor covariance.
 *  - see factor_cov_fn() for more information.
 */
FACTOR_COV (Decay) {
  /* get the summed input values along the factor dimension. */
  const double xp = vector_get(x1, f->d) + vector_get(x2, f->d);

  /* get the factor parameters. */
  const double alpha = vector_get(f->par, P_ALPHA);
  const double beta = vector_get(f->par, P_BETA);

  /* compute and return the expectation. */
  return pow(beta / (beta + xp), alpha);
}

/* Decay_diff_mean(): evaluate the decay factor mean gradient.
 *  - see factor_diff_mean_fn() for more information.
 */
FACTOR_DIFF_MEAN (Decay) {
  /* get the input value along the factor dimension. */
  const double xd = vector_get(x, f->d);

  /* get the factor parameters. */
  const double alpha = vector_get(f->par, P_ALPHA);
  const double beta = vector_get(f->par, P_BETA);

  /* compute intermediate quantities. */
  const double X = beta / (beta + xd);
  const double Y = (alpha * xd) / (beta * beta);

  /* compute the partial derivatives. */
  const double dalpha = pow(X, alpha) * log(X);
  const double dbeta = Y * pow(X, alpha - 1.0);

  /* store the partial derivatives. */
  vector_set(df, P_ALPHA, dalpha);
  vector_set(df, P_BETA, dbeta);
}

/* Decay_diff_var(): evaluate the decay factor variance gradient.
 *  - see factor_diff_var_fn() for more information.
 */
FACTOR_DIFF_VAR (Decay) {
  /* get twice the input value along the factor dimension. */
  const double xp = 2.0 * vector_get(x, f->d);

  /* get the factor parameters. */
  const double alpha = vector_get(f->par, P_ALPHA);
  const double beta = vector_get(f->par, P_BETA);

  /* compute intermediate quantities. */
  const double X = beta / (beta + xp);
  const double Y = (alpha * xp) / (beta * beta);

  /* compute the partial derivatives. */
  const double dalpha = pow(X, alpha) * log(X);
  const double dbeta = Y * pow(X, alpha - 1.0);

  /* store the partial derivatives. */
  vector_set(df, P_ALPHA, dalpha);
  vector_set(df, P_BETA, dbeta);
}

/* Decay_div(): evaluate the decay factor divergence.
 *  - see factor_div_fn() for more information.
 */
FACTOR_DIV (Decay) {
  /* get the first factor parameters. */
  const double alpha = vector_get(f->par, P_ALPHA);
  const double beta = vector_get(f->par, P_BETA);

  /* get the second factor parameters. */
  const double alpha2 = vector_get(f2->par, P_ALPHA);
  const double beta2 = vector_get(f2->par, P_BETA);

  /* compute the divergence. */
  return alpha  * log(beta)  - lgamma(alpha)
       - alpha2 * log(beta2) + lgamma(alpha2)
       + (alpha - alpha2) * (digamma(alpha) - log(beta))
       + (beta - beta2) * (alpha / beta);
}

/* Decay_kernel(): write the kernel code of a decay factor.
 *  - see factor_kernel_fn() for more information.
 */
FACTOR_KERNEL (Decay) {
  /* define the kernel code format string. */
  const char *fmt = "\
const double xd = x1[%u] + x2[%u];\n\
const double alpha = par[%u];\n\
const double beta  = par[%u];\n\
cov = pow(beta / (beta + xd), alpha);\n\
";

  /* allocate and write the kernel code string. */
  char *kstr = malloc(strlen(fmt) + 8);
  if (kstr)
    sprintf(kstr, fmt, f->d, f->d, p0 + P_ALPHA, p0 + P_BETA);

  /* return the new string. */
  return kstr;
}

/* Decay_set(): store a parameter into a decay factor.
 *  - see factor_set_fn() for more information.
 */
FACTOR_SET (Decay) {
  /* determine which parameter is being assigned. */
  switch (i) {
    /* shape parameter: in (0, inf) */
    case P_ALPHA:;
      if (value <= 0.0)
        return 0.0;

      {
        const double alpha = value;
        const double beta = vector_get(f->par, P_BETA);

        vector_set(f->par, i, alpha);

        matrix_set(f->inf, P_ALPHA, P_ALPHA, trigamma(alpha));
        matrix_set(f->inf, P_BETA, P_BETA, alpha / (beta * beta));
      }

      return 1;

    /* rate parameter: in (0, inf) */
    case P_BETA:;
      if (value <= 0.0)
        return 0;

      {
        const double alpha = vector_get(f->par, P_ALPHA);
        const double beta = value;

        vector_set(f->par, i, beta);

        matrix_set(f->inf, P_ALPHA, P_BETA, -1.0 / beta);
        matrix_set(f->inf, P_BETA, P_ALPHA, -1.0 / beta);
        matrix_set(f->inf, P_BETA, P_BETA, alpha / (beta * beta));
      }

      return 1;
  }

  /* invalid parameter index. */
  return 0;
}

/* --- */

/* Decay_new(): allocate a new decay factor.
 *  - see PyTypeObject.tp_new for details.
 */
VFL_TYPE_NEW (Decay) {
  /* allocate a new decay factor. */
  Decay *self = (Decay*) type->tp_alloc(type, 0);
  Factor_reset((Factor*) self);
  if (!self)
    return NULL;

  /* initialize the function pointers. */
  Factor *f = (Factor*) self;
  f->eval      = Decay_eval;
  f->mean      = Decay_mean;
  f->var       = Decay_var;
  f->cov       = Decay_cov;
  f->diff_mean = Decay_diff_mean;
  f->diff_var  = Decay_diff_var;
  f->div       = Decay_div;
  f->kernel    = Decay_kernel;
  f->set       = Decay_set;

  /* resize to the default size. */
  if (!factor_resize(f, 1, 2, 1)) {
    Py_DECREF(f);
    return NULL;
  }

  /* set the default parameter values. */
  factor_set(f, P_ALPHA, 1.0);
  factor_set(f, P_BETA, 1.0);

  /* return the new object. */
  return (PyObject*) self;
}

/* decay factor properties. */
FACTOR_PROP_GETSET (Decay, alpha, P_ALPHA)
FACTOR_PROP_GETSET (Decay, beta,  P_BETA)

/* Decay_getset: property definition structure for decay factors.
 */
static PyGetSetDef Decay_getset[] = {
  FACTOR_PROP (Decay, alpha),
  FACTOR_PROP (Decay, beta),
  { NULL }
};

/* Decay_methods: method definition structure for decay factors.
 */
static PyMethodDef Decay_methods[] = {
  { NULL }
};

/* Decay_Type, Decay_Type_init() */
VFL_TYPE (Decay, Factor, factor)

