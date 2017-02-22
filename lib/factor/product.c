
/* include the factor header. */
#include <vfl/factor.h>

/* factor_product(): allocate a new product factor.
 *
 * arguments:
 *  @F: number of factors in the variable-length argument list.
 *  @...: variable-length argument list containing @F repeats,
 *        having the form (d, factor):
 *   @d: dimension index of the current factor.
 *   @factor: pointer to the current factor.
 *
 * returns:
 *  newly allocated and initialized product factor.
 */
factor_t *factor_product (const unsigned int F, ...) {
  /* declare required variables:
   *  @d: current dimension index.
   *  @D: total number of dimensions.
   *  @P: total number of parameters.
   *  @K: total number of weights.
   *  @factors: array of factor arguments.
   *  @vl: argument list.
   */
  unsigned int d, D, P, K;
  factor_t **factors;
  va_list vl;

  /* return null if zero factors were provided. */
  if (F == 0)
    return NULL;

  /* allocate an array to store factors into. */
  factors = (factor_t**) malloc(F * sizeof(factor_t*));
  if (!factors)
    return NULL;

  /* begin parsing the argument list. */
  va_start(vl, F);
  D = P = K = 0;

  /* loop for the expected number of factors. */
  for (unsigned int f = 0; f < F; f++) {
    /* read the dimension index and the factor pointer. */
    d = va_arg(vl, unsigned int);
    factors[f] = va_arg(vl, factor_t*);

    /* check the factor pointer. */
    if (!factors[f]) {
      free(factors);
      va_end(vl);
      return NULL;
    }

    /* set the argument dimension index. */
    factors[f]->d = d;

    /* update the counts. */
    D = (d + factors[f]->D > D ? d + factors[f]->D : D);
    K = (factors[f]->K > K ? factors[f]->K : K);
    P += factors[f]->P;
  }

  /* end parsing the argument list. */
  va_end(vl);

  /* allocate a factor with extra memory. */
  factor_t *f = factor_alloc(sizeof(product_t), D, P, K);
  if (!f)
    return NULL;

  /* store the expectation function pointers. */
  f->mean = factor_product_mean;
  f->var = factor_product_var;

  /* store the gradient function pointers. */
  f->diff_mean = factor_product_diff_mean;
  f->diff_var = factor_product_diff_var;

  /* store the divergence funciton pointer. */
  f->div = factor_product_div;

  /* store the assignment function pointer. */
  f->set = factor_product_set;
  f->copy = factor_product_copy;
  f->free = factor_product_free;

  /* initialize the combined parameter vector and information matrix. */
  for (unsigned int fidx = 0, p0 = 0; fidx < F; fidx++) {
    /* get the current factor parameter count. */
    const unsigned int Pf = factors[fidx]->P;

    /* get views of the combined vector and matrix. */
    matrix_view_t inf = matrix_submatrix(f->inf, p0, p0, Pf, Pf);
    vector_view_t par = vector_subvector(f->par, p0, Pf);

    /* copy the contents from the factor into the views. */
    matrix_copy(&inf, factors[fidx]->inf);
    vector_copy(&par, factors[fidx]->par);

    /* increment the parameter offset. */
    p0 += Pf;
  }

  /* store the extra structure members. */
  product_t *fx = (product_t*) f;
  fx->factors = factors;
  fx->F = F;

  /* return the new cosine factor. */
  return f;
}

/* factor_product_mean(): evaluate the product factor mean.
 *  - see factor_mean_fn() for more information.
 */
double factor_product_mean (const factor_t *f,
                            const vector_t *x,
                            const unsigned int i) {
  /* get the extended structure pointer. */
  product_t *fx = (product_t*) f;

  /* include the means of each factor. */
  double mean = 1.0;
  for (unsigned int n = 0; n < fx->F; n++) {
    factor_t *fn = fx->factors[n];
    mean *= factor_mean(fn, x, i % fn->K);
  }

  /* return the computed expectation. */
  return mean;
}

/* factor_product_var(): evaluate the product factor variance.
 *  - see factor_var_fn() for more information.
 */
double factor_product_var (const factor_t *f,
                           const vector_t *x,
                           const unsigned int i,
                           const unsigned int j) {
  /* get the extended structure pointer. */
  product_t *fx = (product_t*) f;

  /* include the variances of each factor. */
  double var = 1.0;
  for (unsigned int n = 0; n < fx->F; n++) {
    factor_t *fn = fx->factors[n];
    var *= factor_var(fn, x, i % fn->K, j % fn->K);
  }

  /* return the computed expectation. */
  return var;
}

/* factor_product_diff_mean(): evaluate the product factor mean gradient.
 *  - see factor_diff_mean_fn() for more information.
 */
void factor_product_diff_mean (const factor_t *f,
                               const vector_t *x,
                               const unsigned int i,
                               vector_t *df) {
  /* get the extended structure pointer and factor count. */
  product_t *fx = (product_t*) f;
  const unsigned int F = fx->F;

  /* initialize the output vector with each factor gradient. */
  for (unsigned int n2 = 0, p0 = 0; n2 < F; n2++) {
    /* get the current factor parameter count. */
    factor_t *f2 = fx->factors[n2];
    const unsigned int Pf = f2->P;

    /* store the factor gradient into the appropriate subvector. */
    vector_view_t df2 = vector_subvector(df, p0, Pf);
    factor_diff_mean(f2, x, i % f2->K, &df2);

    /* increment the parameter offset. */
    p0 += Pf;
  }

  /* loop to include "other-factor" means in the output. */
  for (unsigned int n1 = 0; n1 < F; n1++) {
    /* compute the current factor mean. */
    factor_t *f1 = fx->factors[n1];
    const double mean1 = factor_mean(f1, x, i % f1->K);

    /* loop over the factors again. */
    for (unsigned int n2 = 0, p0 = 0; n2 < F; n2++) {
      /* get the current factor parameter count. */
      const unsigned int Pf = fx->factors[n2]->P;

      /* include only "other-factor" contributions. */
      if (n2 != n1) {
        vector_view_t df2 = vector_subvector(df, p0, Pf);
        vector_scale(&df2, mean1);
      }

      /* increment the parameter offset. */
      p0 += Pf;
    }
  }
}

/* factor_product_diff_var(): evaluate the product factor variance gradient.
 *  - see factor_diff_var_fn() for more information.
 */
void factor_product_diff_var (const factor_t *f,
                              const vector_t *x,
                              const unsigned int i,
                              const unsigned int j,
                              vector_t *df) {
  /* get the extended structure pointer and factor count. */
  product_t *fx = (product_t*) f;
  const unsigned int F = fx->F;

  /* initialize the output vector with each factor gradient. */
  for (unsigned int n2 = 0, p0 = 0; n2 < F; n2++) {
    /* get the current factor parameter count. */
    factor_t *f2 = fx->factors[n2];
    const unsigned int Pf = f2->P;

    /* store the factor gradient into the appropriate subvector. */
    vector_view_t df2 = vector_subvector(df, p0, Pf);
    factor_diff_var(f2, x, i % f2->K, j % f2->K, &df2);

    /* increment the parameter offset. */
    p0 += Pf;
  }

  /* loop to include "other-factor" variances in the output. */
  for (unsigned int n1 = 0; n1 < F; n1++) {
    /* compute the current factor variance. */
    factor_t *f1 = fx->factors[n1];
    const double var1 = factor_var(f1, x, i % f1->K, j % f1->K);

    /* loop over the factors again. */
    for (unsigned int n2 = 0, p0 = 0; n2 < F; n2++) {
      /* get the current factor parameter count. */
      const unsigned int Pf = fx->factors[n2]->P;

      /* include only "other-factor" contributions. */
      if (n2 != n1) {
        vector_view_t df2 = vector_subvector(df, p0, Pf);
        vector_scale(&df2, var1);
      }

      /* increment the parameter offset. */
      p0 += Pf;
    }
  }
}

/* factor_product_div(): evaluate the product factor divergence.
 *  - see factor_div_fn() for more information.
 */
double factor_product_div (const factor_t *f, const factor_t *f2) {
  /* get the extended structure pointers. */
  product_t *fx = (product_t*) f;
  product_t *f2x = (product_t*) f2;

  /* sum the divergences of each factor together. */
  double div = 0.0;
  for (unsigned int n = 0; n < fx->F; n++)
    div += factor_div(fx->factors[n], f2x->factors[n]);

  /* return the computed divergence. */
  return div;
}

/* factor_product_set(): store a parameter into a product factor.
 *  - see factor_set_fn() for more information.
 */
int factor_product_set (factor_t *f, const unsigned int i,
                        const double value) {
  /* get the extended structure pointer. */
  product_t *fx = (product_t*) f;

  /* loop over the factor array. */
  for (unsigned int n = 0, p0 = 0; n < fx->F; n++) {
    /* get the current factor. */
    factor_t *fn = fx->factors[n];
    const unsigned int Pn = fn->P;

    /* if the parameter index is now in bounds, set the parameter
     * of the current factor, using the parameter array offset.
     */
    if (i < p0 + Pn) {
      /* attempt to set the factor parameter. */
      if (!factor_set(fn, i - p0, value))
        return 0;

      /* get views of the combined vector and matrix. */
      matrix_view_t inf = matrix_submatrix(f->inf, p0, p0, Pn, Pn);
      vector_view_t par = vector_subvector(f->par, p0, Pn);

      /* copy the contents from the factor into the views. */
      matrix_copy(&inf, fn->inf);
      vector_copy(&par, fn->par);

      /* return success. */
      return 1;
    }

    /* update the parameter offset. */
    p0 += Pn;
  }

  /* invalid parameter index. */
  return 0;
}

/* factor_product_copy(): copy extra information between product factors.
 *  - see factor_copy_fn() for more information.
 */
int factor_product_copy (const factor_t *f, factor_t *fdup) {
  /* get the extended structure pointers. */
  product_t *fdupx = (product_t*) fdup;
  product_t *fx = (product_t*) f;

  /* copy the factor count. */
  const unsigned int F = fx->F;
  fdupx->F = F;

  /* allocate the duplicate factor array. */
  fdupx->factors = (factor_t**) malloc(F * sizeof(factor_t*));
  if (!fdupx->factors)
    return 0;

  /* initialize the duplicate factor array. */
  for (unsigned int i = 0; i < F; i++)
    fdupx->factors[i] = NULL;

  /* copy each factor into the duplicate factor array. */
  for (unsigned int i = 0; i < F; i++) {
    fdupx->factors[i] = factor_copy(fx->factors[i]);
    if (!fdupx->factors[i])
      return 0;
  }

  /* return success. */
  return 1;
}

/* factor_product_free(): free extra information from product factors.
 *  - see factor_free_fn() for more information.
 */
void factor_product_free (factor_t *f) {
  /* get the extended structure pointer. */
  product_t *fx = (product_t*) f;

  /* free each factor in the array. */
  for (unsigned int i = 0; i < fx->F; i++)
    factor_free(fx->factors[i]);

  /* free the array of factors. */
  free(fx->factors);
}

