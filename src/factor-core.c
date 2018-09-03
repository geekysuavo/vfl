
/* include the vfl header. */
#include <vfl/vfl.h>

/* factor_reset(): reset the contents of a factor structure.
 *
 * arguments:
 *  @f: factor structure pointer to modify.
 */
void factor_reset (Factor *f) {
  /* return if the struct pointer is null. */
  if (!f)
    return;

  /* initialize the function pointers. */
  f->eval = NULL;
  f->mean = NULL;
  f->var = NULL;
  f->cov = NULL;
  f->diff_mean = NULL;
  f->diff_var = NULL;
  f->meanfield = NULL;
  f->div = NULL;
  f->init = NULL;
  f->resize = NULL;
  f->kernel = NULL;
  f->set = NULL;
  f->copy = NULL;
  f->free = NULL;

  /* initialize the sizes. */
  f->D = 0;
  f->P = 0;
  f->K = 0;

  /* initialize the input dimension. */
  f->d = 0;

  /* initialize the flags. */
  f->fixed = 0;

  /* initialize the core data. */
  f->inf = NULL;
  f->par = NULL;
}

/* factor_copy(): create a copy of a factor.
 *
 * arguments:
 *  @f: factor structure pointer to access.
 *
 * returns:
 *  pointer to a new factor structure with identical data.
 *  (New reference)
 */
Factor *factor_copy (const Factor *f) {
  /* return null if the input is null. */
  if (!f)
    return NULL;

  /* allocate a new factor having the same type. */
  Factor *g = (Factor*) PyObject_CallObject((PyObject*) Py_TYPE(f), NULL);
  if (!g)
    return NULL;

  /* check if the factors have different sizes. */
  if (g->D != f->D ||
      g->P != f->P ||
      g->K != f->K) {
    /* resize the duplicate to match. */
    if (!factor_resize(g, f->D, f->P, f->K)) {
      Py_DECREF(g);
      return NULL;
    }
  }

  /* copy the factor flags and dimension index. */
  g->fixed = f->fixed;
  g->d = f->d;

  /* copy the information matrix and parameter vector. */
  matrix_copy(g->inf, f->inf);
  vector_copy(g->par, f->par);

  /* if the input factor has a copy function assigned, call it. */
  if (f->copy && !f->copy(f, g)) {
    Py_DECREF(g);
    return NULL;
  }

  /* return the new factor. */
  return g;
}

/* factor_resize(): modify the dimension, parameter and weight counts
 * of a factor.
 *
 * arguments:
 *  @f: factor structure pointer to modify.
 *  @D: new number of dimensions.
 *  @P: new number of parameters.
 *  @K: new number of weights.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int factor_resize (Factor *f, size_t D, size_t P, size_t K) {
  /* check the factor structure pointer. */
  if (!f)
    return 0;

  /* check the dimension and weight counts. these may not be zero. */
  if (!D || !K)
    return 0;

  /* allocate a new information matrix and parameter vector. */
  Matrix *inf = matrix_alloc(P, P);
  Vector *par = vector_alloc(P);

  /* check for allocation failure. */
  if (!inf || !par) {
    matrix_free(inf);
    vector_free(par);
    return 0;
  }

  /* replace the information matrix and parameter vector. */
  matrix_free(f->inf);
  vector_free(f->par);
  f->inf = inf;
  f->par = par;

  /* initialize the matrix and vector elements. */
  matrix_set_zero(f->inf);
  vector_set_zero(f->par);

  /* check for a resize function pointer. */
  if (f->resize && !f->resize(f, D, P, K))
    return 0;

  /* store the new factor sizes. */
  f->D = D;
  f->P = P;
  f->K = K;

  /* return success. */
  return 1;
}

/* factor_dims(): get the number of dimensions supported by a factor.
 *
 * arguments:
 *  @f: factor structure pointer to access.
 *
 * returns:
 *  number of dimensions of the factor.
 */
size_t factor_dims (const Factor *f) {
  /* return the factor dimension count. */
  return (f ? f->D : 0);
}

/* factor_parms(): get the number of parameters in a factor.
 *
 * arguments:
 *  @f: factor structure pointer to access.
 *
 * returns:
 *  number of parameters in the factor.
 */
size_t factor_parms (const Factor *f) {
  /* return the factor parameter count. */
  return (f ? f->P : 0);
}

/* factor_weights(): get the number of weights associated with a factor.
 *
 * arguments:
 *  @f: factor structure pointer to access.
 *
 * returns:
 *  number of weights for the factor.
 */
size_t factor_weights (const Factor *f) {
  /* return the factor weight count. */
  return (f ? f->K : 0);
}

/* factor_get(): extract a parameter from a variational factor.
 *
 * arguments:
 *  @f: factor structure pointer to access.
 *  @i: factor parameter index.
 *
 * returns:
 *  requested parameter value.
 */
double factor_get (const Factor *f, size_t i) {
  /* check the input pointer and parameter index. */
  if (!f || i >= f->P)
    return 0.0;

  /* return the parameter vector element. */
  return vector_get(f->par, i);
}

/* factor_set(): store a parameter into a variational factor.
 *
 * arguments:
 *  @f: factor structure pointer to modify.
 *  @i: factor parameter index.
 *  @value: parameter value.
 *
 * returns:
 *  integer indicating assignment success (1) or failure (0).
 */
int factor_set (Factor *f, size_t i, double value) {
  /* check the input pointer and parameter index. */
  if (!f || i >= f->P)
    return 0;

  /* check the function pointer. */
  if (!f->set)
    return 0;

  /* execute the parameter assignment function. */
  return f->set(f, i, value);
}

/* factor_fix(): set the fixed flag of a variational factor.
 *
 * arguments:
 *  @f: factor structure pointer to modify.
 *  @fixed: whether or not the factor is fixed.
 */
void factor_fix (Factor *f, int fixed) {
  /* return if the input pointer is null. */
  if (!f)
    return;

  /* set the fixed flag of the factor. */
  f->fixed = fixed;
}

/* factor_eval(): evaluate a factor at its mode.
 *  - see factor_mean_fn() for more information.
 */
double factor_eval (const Factor *f, const Vector *x,
                    size_t p, size_t i) {
  /* check the input pointers and the basis index. */
  if (!f || !x || i >= f->K)
    return 0.0;

  /* check the function pointer. */
  if (!f->eval)
    return 0.0;

  /* execute the evaluation function. */
  return f->eval(f, x, p, i);
}

/* factor_mean(): evaluate the mean function of a factor.
 *  - see factor_mean_fn() for more information.
 */
double factor_mean (const Factor *f, const Vector *x,
                    size_t p, size_t i) {
  /* check the input pointers and the basis index. */
  if (!f || !x || i >= f->K)
    return 0.0;

  /* check the function pointer. */
  if (!f->mean)
    return 0.0;

  /* execute the mean function. */
  return f->mean(f, x, p, i);
}

/* factor_var(): evaluate the variance function of a factor.
 *  - see factor_var_fn() for more information.
 */
double factor_var (const Factor *f, const Vector *x,
                   size_t p, size_t i, size_t j) {
  /* check the input pointers and basis indices. */
  if (!f || !x || i >= f->K || j >= f->K)
    return 0.0;

  /* check the function pointer. */
  if (!f->var)
    return 0.0;

  /* execute the variance function. */
  return f->var(f, x, p, i, j);
}

/* factor_cov(): evaluate the covariance function of a factor.
 *  - see factor_cov_fn() for more information.
 */
double factor_cov (const Factor *f, const Vector *x1, const Vector *x2,
                   size_t p1, size_t p2) {
  /* check the input pointers. */
  if (!f || !x1 || !x2)
    return 0.0;

  /* check the function pointer. */
  if (!f->cov)
    return 0.0;

  /* execute the covariance function. */
  return f->cov(f, x1, x2, p1, p2);
}

/* factor_diff_mean(): evaluate the mean gradient function of a factor.
 *  - see factor_diff_mean_fn() for more information.
 */
int factor_diff_mean (const Factor *f, const Vector *x,
                      size_t p, size_t i, Vector *df) {
  /* check the input pointers, basis index, and gradient length. */
  if (!f || !x || !df || i >= f->K || df->len != f->P)
    return 0;

  /* return zero gradient if the factor is fixed. */
  if (f->fixed) {
    vector_set_zero(df);
    return 1;
  }

  /* check the function pointer. */
  if (!f->diff_mean)
    return 0;

  /* execute the mean gradient function and return success. */
  f->diff_mean(f, x, p, i, df);
  return 1;
}

/* factor_diff_var(): evaluate the variance gradient function of a factor.
 *  - see factor_diff_var_fn() for more information.
 */
int factor_diff_var (const Factor *f, const Vector *x,
                     size_t p, size_t i, size_t j,
                     Vector *df) {
  /* check the input pointers, basis indices, and gradient length. */
  if (!f || !x || !df || i >= f->K || j >= f->K || df->len != f->P)
    return 0;

  /* return zero gradient if the factor is fixed. */
  if (f->fixed) {
    vector_set_zero(df);
    return 1;
  }

  /* check the function pointer. */
  if (!f->diff_var)
    return 0;

  /* execute the variance gradient function and return success. */
  f->diff_var(f, x, p, i, j, df);
  return 1;
}

/* factor_meanfield(): perform an assumed-density mean-field factor update.
 *  - see factor_meanfield_fn() for more information.
 */
int factor_meanfield (Factor *f, const Factor *fp, const Datum *dat,
                      Vector *b, Matrix *B) {
  /* check the input pointer. */
  if (!f)
    return 0;

  /* perform no update if the factor is fixed. */
  if (f->fixed)
    return 1;

  /* check the function pointer. */
  if (!f->meanfield)
    return 0;

  /* execute the mean-field update function. */
  return f->meanfield(f, fp, dat, b, B);
}

/* factor_div(): evaluate the divergence function of a factor.
 *  - see factor_div_fn() for more information.
 */
double factor_div (const Factor *f, const Factor *f2) {
  /* check the input pointers for compatibility. */
  if (!f || !f2 || f->P != f2->P)
    return 0.0;

  /* check the function pointer. */
  if (!f->div)
    return 0.0;

  /* execute the divergence function. */
  return f->div(f, f2);
}

/* factor_kernel(): write covariance kernel code of a factor.
 *  - see factor_kernel_fn() for more information.
 */
char *factor_kernel (const Factor *f, size_t p0) {
  /* check the input pointer. */
  if (!f)
    return NULL;

  /* check the function pointer. */
  if (!f->kernel)
    return NULL;

  /* execute the kernel function. */
  return f->kernel(f, p0);
}

