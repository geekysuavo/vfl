
/* include the factor header. */
#include <vfl/factor.h>

/* product_t: structure for holding a product factor.
 */
typedef struct {
  /* @base: core factor structure members. */
  factor_t base;

  /* @factors: array of factors in the product.
   * @F: number of factors in the product.
   */
  factor_t **factors;
  unsigned int F;
}
product_t;

/* product_init(): initialize the product factor structure.
 *  - see factor_init_fn() for more information.
 */
/* FIXME: product factors are now broken! */
FACTOR_INIT (product) {
  /* initialize the factors array. */
  product_t *fx = (product_t*) f;
  fx->factors = NULL;
  fx->F = 0;

  /* return success. */
  return 1;
}

/* product_mean(): evaluate the product factor mean.
 *  - see factor_mean_fn() for more information.
 */
FACTOR_MEAN (product) {
  /* get the extended structure pointer. */
  product_t *fx = (product_t*) f;

  /* include the means of each factor. */
  double mean = 1.0;
  for (unsigned int n = 0; n < fx->F; n++) {
    factor_t *fn = fx->factors[n];
    mean *= factor_mean(fn, x, p, i % fn->K);
  }

  /* return the computed expectation. */
  return mean;
}

/* product_var(): evaluate the product factor variance.
 *  - see factor_var_fn() for more information.
 */
FACTOR_VAR (product) {
  /* get the extended structure pointer. */
  product_t *fx = (product_t*) f;

  /* include the variances of each factor. */
  double var = 1.0;
  for (unsigned int n = 0; n < fx->F; n++) {
    factor_t *fn = fx->factors[n];
    var *= factor_var(fn, x, p, i % fn->K, j % fn->K);
  }

  /* return the computed expectation. */
  return var;
}

/* product_diff_mean(): evaluate the product factor mean gradient.
 *  - see factor_diff_mean_fn() for more information.
 */
FACTOR_DIFF_MEAN (product) {
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
    factor_diff_mean(f2, x, p, i % f2->K, &df2);

    /* increment the parameter offset. */
    p0 += Pf;
  }

  /* loop to include "other-factor" means in the output. */
  for (unsigned int n1 = 0; n1 < F; n1++) {
    /* compute the current factor mean. */
    factor_t *f1 = fx->factors[n1];
    const double mean1 = factor_mean(f1, x, p, i % f1->K);

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

/* product_diff_var(): evaluate the product factor variance gradient.
 *  - see factor_diff_var_fn() for more information.
 */
FACTOR_DIFF_VAR (product) {
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
    factor_diff_var(f2, x, p, i % f2->K, j % f2->K, &df2);

    /* increment the parameter offset. */
    p0 += Pf;
  }

  /* loop to include "other-factor" variances in the output. */
  for (unsigned int n1 = 0; n1 < F; n1++) {
    /* compute the current factor variance. */
    factor_t *f1 = fx->factors[n1];
    const double var1 = factor_var(f1, x, p, i % f1->K, j % f1->K);

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

/* product_div(): evaluate the product factor divergence.
 *  - see factor_div_fn() for more information.
 */
FACTOR_DIV (product) {
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

/* product_set(): store a parameter into a product factor.
 *  - see factor_set_fn() for more information.
 */
FACTOR_SET (product) {
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

/* product_copy(): copy extra information between product factors.
 *  - see factor_copy_fn() for more information.
 */
FACTOR_COPY (product) {
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

/* product_free(): free extra information from product factors.
 *  - see factor_free_fn() for more information.
 */
FACTOR_FREE (product) {
  /* get the extended structure pointer. */
  product_t *fx = (product_t*) f;

  /* free each factor in the array. */
  for (unsigned int i = 0; i < fx->F; i++)
    factor_free(fx->factors[i]);

  /* free the array of factors. */
  free(fx->factors);
}

/* product_type: product factor type structure.
 */
static factor_type_t product_type = {
  "product",                                     /* name      */
  sizeof(product_t),                             /* size      */
  0,                                             /* initial D */
  0,                                             /* initial P */
  0,                                             /* initial K */
  product_mean,                                  /* mean      */
  product_var,                                   /* var       */
  product_diff_mean,                             /* diff_mean */
  product_diff_var,                              /* diff_var  */
  NULL,                                          /* meanfield */
  product_div,                                   /* div       */
  product_init,                                  /* init      */
  product_set,                                   /* set       */
  product_copy,                                  /* copy      */
  product_free                                   /* free      */
};

/* factor_type_product: address of the product_type
 * structure.
 */
const factor_type_t *factor_type_product = &product_type;

