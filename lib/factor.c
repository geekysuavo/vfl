
/* include the factor header. */
#include <vfl/factor.h>

/* factor_alloc(): allocate a new variational factor.
 *
 * arguments:
 *  @type: pointer to a factor type structure.
 *
 * returns:
 *  newly allocated and initialized factor structure pointer.
 */
factor_t *factor_alloc (const factor_type_t *type) {
  /* check that the type structure is valid. */
  if (!type)
    return NULL;

  /* allocate the structure pointer. */
  factor_t *f = malloc(type->size);
  if (!f)
    return NULL;

  /* initialize the factor type. */
  f->type = *type;

  /* set the default factor flags and dimension. */
  f->fixed = 0;
  f->d = 0;

  /* initialize the information matrix and parameter vector. */
  f->inf = NULL;
  f->par = NULL;

  /* initialize the factor sizes from their defaults. */
  if (!factor_resize(f, type->D, type->P, type->K)) {
    /* failed to resize. */
    factor_free(f);
    return NULL;
  }

  /* execute the initialization function, if defined. */
  factor_init_fn init_fn = FACTOR_TYPE(f)->init;
  if (init_fn && !init_fn(f)) {
    /* initialization failed. */
    factor_free(f);
    return NULL;
  }

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
  factor_t *fdup = factor_alloc(&f->type);
  if (!fdup)
    return NULL;

  /* resize the new factor to match the input factor. */
  if (!factor_resize(fdup, f->D, f->P, f->K)) {
    factor_free(fdup);
    return NULL;
  }

  /* copy the factor flags and dimension. */
  fdup->fixed = f->fixed;
  fdup->d = f->d;

  /* copy the information matrix and parameter vector. */
  matrix_copy(fdup->inf, f->inf);
  vector_copy(fdup->par, f->par);

  /* if the input factor has a copy function assigned, execute it. */
  factor_copy_fn copy_fn = FACTOR_TYPE(f)->copy;
  if (copy_fn && !copy_fn(f, fdup)) {
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
  factor_free_fn free_fn = FACTOR_TYPE(f)->free;
  if (free_fn)
    free_fn(f);

  /* free the information matrix and parameter vector. */
  matrix_free(f->inf);
  vector_free(f->par);

  /* free the structure pointer. */
  free(f);
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
int factor_resize (factor_t *f,
                   const unsigned int D,
                   const unsigned int P,
                   const unsigned int K) {
  /* check the factor structure pointer. */
  if (!f)
    return 0;

  /* check the dimension and weight counts. these may not be zero. */
  if (!D || !K)
    return 0;

  /* free the information matrix and parameter vector. */
  matrix_free(f->inf);
  vector_free(f->par);
  f->inf = NULL;
  f->par = NULL;

  /* if the factor has parameters... */
  if (P) {
    /* ... allocate the information matrix and parameter vector. */
    f->inf = matrix_alloc(P, P);
    f->par = vector_alloc(P);

    /* check for allocation failure. */
    if (!f->inf || !f->par)
      return 0;
  }

  /* initialize the matrix and vector elements. */
  matrix_set_zero(f->inf);
  vector_set_zero(f->par);

  /* store the new factor sizes. */
  f->D = D;
  f->P = P;
  f->K = K;

  /* return success. */
  return 1;
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
  /* check the input pointer and parameter index. */
  if (!f || i >= f->P)
    return 0;

  /* check the function pointer. */
  factor_set_fn set_fn = FACTOR_TYPE(f)->set;
  if (!set_fn)
    return 0;

  /* execute the parameter assignment function. */
  return set_fn(f, i, value);
}

/* factor_mean(): evaluate the mean function of a factor.
 *  - see factor_mean_fn() for more information.
 */
double factor_mean (const factor_t *f,
                    const vector_t *x,
                    const unsigned int p,
                    const unsigned int i) {
  /* check the input pointers and the basis index. */
  if (!f || !x || i >= f->K)
    return 0.0;

  /* check the function pointer. */
  factor_mean_fn mean_fn = FACTOR_TYPE(f)->mean;
  if (!mean_fn)
    return 0.0;

  /* execute the mean function. */
  return mean_fn(f, x, p, i);
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
  if (!f || !x || i >= f->K || j >= f->K)
    return 0.0;

  /* check the function pointer. */
  factor_var_fn var_fn = FACTOR_TYPE(f)->var;
  if (!var_fn)
    return 0.0;

  /* execute the variance function. */
  return var_fn(f, x, p, i, j);
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
  if (!f || !x || !df || i >= f->K || df->len != f->P)
    return 0;

  /* check the function pointer. */
  factor_diff_mean_fn diff_mean_fn = FACTOR_TYPE(f)->diff_mean;
  if (!diff_mean_fn)
    return 0;

  /* execute the mean gradient function and return success. */
  diff_mean_fn(f, x, p, i, df);
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
  if (!f || !x || !df || i >= f->K || j >= f->K || df->len != f->P)
    return 0;

  /* check the function pointer. */
  factor_diff_var_fn diff_var_fn = FACTOR_TYPE(f)->diff_var;
  if (!diff_var_fn)
    return 0;

  /* execute the variance gradient function and return success. */
  diff_var_fn(f, x, p, i, j, df);
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

  /* check the function pointer. */
  factor_meanfield_fn meanfield_fn = FACTOR_TYPE(f)->meanfield;
  if (!meanfield_fn)
    return 0;

  /* execute the mean-field update function. */
  return meanfield_fn(f, fp, dat, c, C);
}

/* factor_div(): evaluate the divergence function of a factor.
 *  - see factor_div_fn() for more information.
 */
double factor_div (const factor_t *f, const factor_t *f2) {
  /* check the input pointers for compatibility. */
  if (!f || !f2 || f->P != f2->P)
    return 0.0;

  /* check the function pointer. */
  factor_div_fn div_fn = FACTOR_TYPE(f)->div;
  if (!div_fn)
    return 0.0;

  /* execute the divergence function. */
  return div_fn(f, f2);
}

