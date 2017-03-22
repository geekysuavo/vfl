
/* include the factor header. */
#include <vfl/factor.h>

/* factor_alloc(): allocate a new variational factor.
 *
 * arguments:
 *  @bytes: amount of memory to allocate, or zero for the default.
 *  @D: number of factor dimensions.
 *  @P: number of factor parameters.
 *  @K: number of associated weights.
 *
 * returns:
 *  newly allocated and initialized factor structure pointer.
 */
factor_t *factor_alloc (const unsigned int bytes,
                        const unsigned int D,
                        const unsigned int P,
                        const unsigned int K) {
  /* determine the amount of memory to allocate. */
  const unsigned int sz = (bytes ? bytes : sizeof(factor_t));

  /* allocate the structure pointer. */
  factor_t *f = (factor_t*) malloc(sz);
  if (!f)
    return NULL;

  /* store the factor sizes. */
  f->bytes = sz;
  f->D = D;
  f->P = P;
  f->K = K;

  /* set the default factor flags. */
  f->fixed = 0;

  /* set the default factor dimension. */
  f->d = 0;

  /* initialize the expectation function pointers. */
  f->mean = NULL;
  f->var = NULL;

  /* initialize the gradient function pointers. */
  f->diff_mean = NULL;
  f->diff_var = NULL;

  /* initialize the mean-field function pointer. */
  f->meanfield = NULL;

  /* initialize the divergence function pointer. */
  f->div = NULL;

  /* initialize the maintenance function pointers. */
  f->set = NULL;
  f->copy = NULL;
  f->free = NULL;

  /* allocate the information matrix and parameter vector. */
  f->inf = matrix_alloc(P, P);
  f->par = vector_alloc(P);

  /* check that the matrix and vector were allocated successfully. */
  if (!f->inf || !f->par) {
    factor_free(f);
    return NULL;
  }

  /* initialize the matrix and vector elements. */
  matrix_set_zero(f->inf);
  vector_set_zero(f->par);

  /* return the new factor. */
  return f;
}

/* factor_copy(): create a deep copy of an allocated factor.
 *
 * arguments:
 *  @f: factor structure pointer to access.
 *
 * returns:
 *  fresh clone of the input factor structure.
 */
factor_t *factor_copy (const factor_t *f) {
  /* return null if the structure pointer is null. */
  if (!f)
    return NULL;

  /* allocate a new factor structure of the same size. */
  factor_t *fdup = factor_alloc(f->bytes, f->D, f->P, f->K);
  if (!fdup)
    return NULL;

  /* copy the factor flags. */
  fdup->fixed = f->fixed;

  /* copy the expectation function pointers. */
  fdup->mean = f->mean;
  fdup->var = f->var;

  /* copy the gradient function pointers. */
  fdup->diff_mean = f->diff_mean;
  fdup->diff_var = f->diff_var;

  /* copy the divergence function pointer. */
  fdup->div = f->div;

  /* copy the maintenance function pointers. */
  fdup->set = f->set;
  fdup->copy = f->copy;
  fdup->free = f->free;

  /* copy the information matrix and parameter vector. */
  matrix_copy(fdup->inf, f->inf);
  vector_copy(fdup->par, f->par);

  /* if the input factor has a copy function assigned, execute it. */
  if (f->copy && !f->copy(f, fdup)) {
    factor_free(fdup);
    return NULL;
  }

  /* return the duplicate factor. */
  return fdup;
}

/* factor_free(): free an allocated factor.
 *
 * arguments:
 *  @f: factor structure pointer to free.
 */
void factor_free (factor_t *f) {
  /* return if the structure pointer is null. */
  if (!f)
    return;

  /* if the factor has a free function assigned, execute it. */
  if (f->free)
    f->free(f);

  /* free the information matrix and parameter vector. */
  matrix_free(f->inf);
  vector_free(f->par);

  /* free the structure pointer. */
  free(f);
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
double factor_get (const factor_t *f, const unsigned int i) {
  /* check the input pointer and parameter index. */
  if (!f || i >= f->P)
    return 0.0;

  /* return the parameter vector element. */
  return vector_get(f->par, i);
}

/* factor_set(): store a parameter into a variational factor.
 *
 * arguments:
 *  @f: factor structure pointer to access.
 *  @i: factor parameter index.
 *  @value: parameter value.
 *
 * returns:
 *  integer indicating assignment success (1) or failure (0).
 */
int factor_set (factor_t *f, const unsigned int i, const double value) {
  /* check the input pointers and parameter index. */
  if (!f || !f->set || i >= f->P)
    return 0;

  /* execute the parameter assignment function. */
  return f->set(f, i, value);
}

/* factor_mean(): evaluate the mean function of a factor.
 *  - see factor_mean_fn() for more information.
 */
double factor_mean (const factor_t *f,
                    const vector_t *x,
                    const unsigned int p,
                    const unsigned int i) {
  /* check the input pointers and the basis index. */
  if (!f || !f->mean || !x || i >= f->K)
    return 0.0;

  /* execute the mean function. */
  return f->mean(f, x, p, i);
}

/* factor_var(): evaluate the variance function of a factor.
 *  - see factor_var_fn() for more information.
 */
double factor_var (const factor_t *f,
                   const vector_t *x,
                   const unsigned int p,
                   const unsigned int i,
                   const unsigned int j) {
  /* check the input pointers and basis indices. */
  if (!f || !f->var || !x || i >= f->K || j >= f->K)
    return 0.0;

  /* execute the variance function. */
  return f->var(f, x, p, i, j);
}

/* factor_diff_mean(): evaluate the mean gradient function of a factor.
 *  - see factor_diff_mean_fn() for more information.
 */
int factor_diff_mean (const factor_t *f,
                      const vector_t *x,
                      const unsigned int p,
                      const unsigned int i,
                      vector_t *df) {
  /* check the input pointers, basis index, and gradient length. */
  if (!f || !f->diff_mean || !x || !df ||
      i >= f->K || df->len != f->P)
    return 0;

  /* execute the mean gradient function and return success. */
  f->diff_mean(f, x, p, i, df);
  return 1;
}

/* factor_diff_var(): evaluate the variance gradient function of a factor.
 *  - see factor_diff_var_fn() for more information.
 */
int factor_diff_var (const factor_t *f,
                     const vector_t *x,
                     const unsigned int p,
                     const unsigned int i,
                     const unsigned int j,
                     vector_t *df) {
  /* check the input pointers, basis indices, and gradient length. */
  if (!f || !f->diff_var || !x || !df ||
      i >= f->K || j >= f->K ||
      df->len != f->P)
    return 0;

  /* execute the variance gradient function and return success. */
  f->diff_var(f, x, p, i, j, df);
  return 1;
}

/* factor_meanfield(): perform an assumed-density mean-field factor update.
 *  - see factor_meanfield_fn() for more information.
 */
int factor_meanfield (factor_t *f, const factor_t *fp, const data_t *dat,
                      const vector_t *c, const matrix_t *C) {
  /* check the input pointers. */
  if (!f || !fp || !dat || !c || !C)
    return 0;

  /* execute the mean-field update function. */
  return f->meanfield(f, fp, dat, c, C);
}

/* factor_div(): evaluate the divergence function of a factor.
 *  - see factor_div_fn() for more information.
 */
double factor_div (const factor_t *f, const factor_t *f2) {
  /* check the input pointers for compatibility. */
  if (!f || !f->div || !f2 || f->P != f2->P)
    return 0.0;

  /* execute the divergence function. */
  return f->div(f, f2);
}

