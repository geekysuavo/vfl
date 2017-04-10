
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

/* product_cov(): evaluate the product factor covariance.
 *  - see factor_cov_fn() for more information.
 */
FACTOR_COV (product) {
  /* get the extended structure pointer. */
  product_t *fx = (product_t*) f;

  /* include the covariances of each factor. */
  double cov = 1.0;
  for (unsigned int n = 0; n < fx->F; n++) {
    factor_t *fn = fx->factors[n];
    cov *= factor_cov(fn, x1, x2, p1, p2);
  }

  /* return the computed expectation. */
  return cov;
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

/* product_kernel(): write the kernel code of a product factor.
 *  - see factor_kernel_fn() for more information.
 */
FACTOR_KERNEL (product) {
  /* get the extended structure pointer. */
  product_t *fx = (product_t*) f;

  /* define kernel code format strings. */
  const char *fmtA = "float prod = 1.0;\n";
  const char *fmtB = "{\n%s}\nprod *= cov;\n";
  const char *fmtC = "cov = prod;\n";

  /* allocate an array for storing sub-factor kernel code. */
  char **fstr = malloc(fx->F * sizeof(char*));
  if (!fstr)
    return NULL;

  /* get the strings of each sub-factor. */
  for (unsigned int n = 0, pn = p0; n < fx->F; n++) {
    /* get the current sub-factor string. */
    const factor_t *fn = fx->factors[n];
    fstr[n] = factor_kernel(fn, pn);

    /* check for failure. */
    if (!fstr[n])
      return NULL;

    /* advance the sub-factor parameter offset. */
    pn += fn->P;
  }

  /* determine the length of the kernel code string. */
  unsigned int len = strlen(fmtA) + strlen(fmtC) + 8;
  for (unsigned int n = 0; n < fx->F; n++)
    len += strlen(fmtB) + strlen(fstr[n]);

  /* allocate the kernel code string. */
  char *kstr = malloc(len);
  if (!kstr)
    return NULL;

  /* write the header. */
  char *pos = kstr;
  pos += sprintf(pos, fmtA);

  /* write each sub-factor string. */
  for (unsigned int n = 0; n < fx->F; n++)
    pos += sprintf(pos, fmtB, fstr[n]);

  /* write the footer. */
  sprintf(pos, fmtC);

  /* free the sub-factor strings. */
  for (unsigned int n = 0; n < fx->F; n++)
    free(fstr[n]);

  /* free the sub-factor string array. */
  free(fstr);

  /* return the new string. */
  return kstr;
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

  /* allocate the duplicate factor array and parameter names table. */
  fdupx->factors = malloc(F * sizeof(factor_t*));
  fdup->parnames = malloc(f->P * sizeof(char*));
  if (!fdupx->factors || !fdup->parnames)
    return 0;

  /* initialize the duplicate parameter names table. */
  for (unsigned int p = 0; p < f->P; p++) {
    const char *parname = factor_parname(f, p);
    fdup->parnames[p] = malloc(strlen(parname) + 2);
    if (fdup->parnames[p])
      strcpy(fdup->parnames[p], parname);
  }

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

/* product_add_factor(): add a new factor to a product factor.
 *
 * arguments:
 *  @f: factor structure pointer.
 *  @d: new factor dimension.
 *  @fd: new factor pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int product_add_factor (factor_t *f, const unsigned int d, factor_t *fd) {
  /* get the extended structure pointer. */
  product_t *fx = (product_t*) f;

  /* get the current factor sizes. */
  unsigned int D = f->D;
  unsigned int P = f->P;
  unsigned int K = f->K;
  unsigned int F = fx->F;

  /* update the factor sizes. */
  D = (d + fd->D > D ? d + fd->D : D);
  K = (fd->K > K ? fd->K : K);
  P += fd->P;
  F++;

  /* resize the base factor. */
  if (!factor_resize(f, D, P, K))
    return 0;

  /* reallocate the parameter names table. */
  f->parnames = realloc(f->parnames, P * sizeof(char*));
  if (!f->parnames)
    return 0;

  /* reallocate the factors array. */
  fx->factors = realloc(fx->factors, F * sizeof(factor_t*));
  if (!fx->factors)
    return 0;

  /* store the new factor and updated factor count. */
  fx->factors[F - 1] = fd;
  fx->F = F;

  /* set the argument dimension index. */
  fd->d = d;

  /* initialize the combined information matrix and parameter vector. */
  for (unsigned int fidx = 0, p0 = 0; fidx < F; fidx++) {
    /* get the current factor parameter count. */
    const unsigned int Pf = fx->factors[fidx]->P;
    if (Pf == 0)
      continue;

    /* get views of the combined matrix and vector. */
    matrix_view_t inf = matrix_submatrix(f->inf, p0, p0, Pf, Pf);
    vector_view_t par = vector_subvector(f->par, p0, Pf);

    /* copy the contents from the factor into the views. */
    matrix_copy(&inf, fx->factors[fidx]->inf);
    vector_copy(&par, fx->factors[fidx]->par);

    /* initialize the new parameter names. */
    if (fidx == F - 1) {
      for (unsigned int p = 0; p < Pf; p++) {
        /* determine the new string length. */
        unsigned int len = strlen(FACTOR_TYPE(fd)->name);
        const char *parname = factor_parname(fd, p);
        len += (parname ? strlen(parname) : 0);
        len += 32;

        /* allocate and build the new string. */
        f->parnames[p0 + p] = malloc(len);
        if (f->parnames[p0 + p])
          snprintf(f->parnames[p0 + p], len - 1, "%s%u.%s",
                   FACTOR_TYPE(fd)->name, fidx, parname);
      }
    }

    /* increment the parameter offset. */
    p0 += Pf;
  }

  /* return success. */
  return 1;
}

/* product_type: product factor type structure.
 */
static factor_type_t product_type = {
  "product",                                     /* name      */
  sizeof(product_t),                             /* size      */
  1,                                             /* initial D */
  0,                                             /* initial P */
  1,                                             /* initial K */
  NULL,                                          /* parnames  */
  product_mean,                                  /* mean      */
  product_var,                                   /* var       */
  product_cov,                                   /* cov       */
  product_diff_mean,                             /* diff_mean */
  product_diff_var,                              /* diff_var  */
  NULL,                                          /* meanfield */
  product_div,                                   /* div       */
  product_init,                                  /* init      */
  NULL,                                          /* resize    */
  product_kernel,                                /* kernel    */
  product_set,                                   /* set       */
  product_copy,                                  /* copy      */
  product_free                                   /* free      */
};

/* factor_type_product: address of the product_type
 * structure.
 */
const factor_type_t *factor_type_product = &product_type;

