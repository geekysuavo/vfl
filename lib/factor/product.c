
/* include the factor header. */
#include <vfl/factor.h>

/* include the required object headers. */
#include <vfl/base/int.h>
#include <vfl/base/map.h>
#include <vfl/base/list.h>

/* product_t: structure for holding a product factor.
 */
typedef struct {
  /* @base: core factor structure members. */
  factor_t base;

  /* product sub-factors:
   *  @factors: array of factors in the product.
   *  @F: number of factors in the product.
   */
  factor_t **factors;
  unsigned int F;

  /* mean-field update variables:
   *  @b0: backup vector of coefficients.
   *  @B0: backup matrix of coefficients.
   */
  vector_t *b0;
  matrix_t *B0;
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

  /* initialize the mean-field variables. */
  fx->b0 = NULL;
  fx->B0 = NULL;

  /* return success. */
  return 1;
}

/* product_eval(): evaluate the product factor at its mode.
 *  - see factor_mean_fn() for more information.
 */
FACTOR_EVAL (product) {
  /* get the extended structure pointer. */
  product_t *fx = (product_t*) f;

  /* include the values of each factor. */
  double mode = 1.0;
  for (unsigned int n = 0; n < fx->F; n++) {
    factor_t *fn = fx->factors[n];
    mode *= factor_eval(fn, x, p, i % fn->K);
  }

  /* return the computed mode. */
  return mode;
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
        blas_dscal(mean1, &df2);
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
        blas_dscal(var1, &df2);
      }

      /* increment the parameter offset. */
      p0 += Pf;
    }
  }
}

/* product_meanfield(): perform a mean-field update of a product factor.
 *  - see factor_meanfield_fn() for more information.
 */
FACTOR_MEANFIELD (product) {
  /* get the extended structure pointers. */
  product_t *fx = (product_t*) f;
  product_t *fpx = (product_t*) fp;

  /* check for initialization calls. */
  if (FACTOR_MEANFIELD_INIT) {
    /* initialize all sub-factors. */
    unsigned int ret = 1;
    for (unsigned int n = 0; n < fx->F; n++)
      ret &= factor_meanfield(fx->factors[n], NULL, NULL, NULL, NULL);

    /* return the result of the initializations. */
    return ret;
  }

  /* check for finalization calls. */
  if (FACTOR_MEANFIELD_END) {
    /* finalize all sub-factors. */
    unsigned int ret = 1;
    for (unsigned int n = 0; n < fx->F; n++)
      ret &= factor_meanfield(fx->factors[n], fpx->factors[n],
                              NULL, NULL, NULL);

    /* update the parameter vector and information matrix. */
    if (ret) product_update(f);

    /* return the result of the finalizations. */
    return ret;
  }

  /* get the sub-factor coefficients. */
  vector_t *bn = fx->b0;
  matrix_t *Bn = fx->B0;

  /* loop over the sub-factors. */
  unsigned int ret = 1;
  for (unsigned int n = 0; n < fx->F; n++) {
    /* get the current sub-factor and sub-prior. */
    factor_t *fn = fx->factors[n];
    factor_t *fpn = fpx->factors[n];

    /* initialize the coefficients. */
    vector_copy(bn, b);
    matrix_copy(Bn, B);

    /* include the expectations from the other sub-factors. */
    for (unsigned int n2 = 0; n2 < fx->F; n2++) {
      /* skip the current sub-factor. */
      if (n2 == n) continue;

      /* adjust the coefficient vector. */
      const factor_t *fn2 = fx->factors[n2];
      for (unsigned int k = 0; k < f->K; k++) {
        const double phi1 = factor_mean(fn2, dat->x, dat->p, k % fn2->K);
        vector_set(bn, k, vector_get(bn, k) * phi1);
      }

      /* adjust the coefficient matrix. */
      for (unsigned int k = 0; k < f->K; k++) {
        for (unsigned int k2 = 0; k2 < f->K; k2++) {
          const double phi2 = factor_var(fn2, dat->x, dat->p,
                                         k % fn2->K, k2 % fn2->K);
          matrix_set(Bn, k, k2, matrix_get(Bn, k, k2) * phi2);
        }
      }
    }

    /* execute the sub-factor meanfield function. */
    ret &= factor_meanfield(fn, fpn, dat, bn, Bn);
  }

  /* return success. */
  return 1;
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

/* product_resize(): handle resizes of the product factor.
 *  - see factor_resize_fn() for more information.
 */
FACTOR_RESIZE (product) {
  /* get the extended structure pointer. */
  product_t *fx = (product_t*) f;

  /* free the mean-field variables. */
  vector_free(fx->b0);
  matrix_free(fx->B0);

  /* allocate new mean-field variables. */
  fx->b0 = vector_alloc(K);
  fx->B0 = matrix_alloc(K, K);
  if (!fx->b0 || !fx->B0)
    return 0;

  /* return success. */
  return 1;
}

/* product_kernel(): write the kernel code of a product factor.
 *  - see factor_kernel_fn() for more information.
 */
FACTOR_KERNEL (product) {
  /* get the extended structure pointer. */
  product_t *fx = (product_t*) f;

  /* define kernel code format strings. */
  const char *fmtA = "double prod = 1.0;\n";
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
    fdupx->factors[i] = (factor_t*) obj_copy((object_t*) fx->factors[i]);
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

  /* free the mean-field variables. */
  vector_free(fx->b0);
  matrix_free(fx->B0);
}

/* product_get_size(): get the number of factors in a product factor.
 *
 * arguments:
 *  @f: factor structure pointer.
 *
 * returns:
 *  number of child factors in the product.
 */
unsigned int product_get_size (const factor_t *f) {
  /* get the extended structure pointer. */
  product_t *fx = (product_t*) f;
  return (f ? fx->F : 0);
}

/* product_get_factor(): get a factor from a product factor.
 *
 * arguments:
 *  @f: factor structure pointer.
 *  @idx: factor index.
 *
 * returns:
 *  requested child factor in the product.
 */
factor_t *product_get_factor (const factor_t *f, const unsigned int idx) {
  /* return the requested factor. */
  product_t *fx = (product_t*) f;
  return (f && idx < fx->F ? fx->factors[idx] : NULL);
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

  /* initialize the information matrix and parameter vector. */
  product_update(f);

  /* initialize the parameter names. */
  for (unsigned int fidx = 0, p0 = 0; fidx < F; fidx++) {
    /* get the current factor parameter count. */
    const unsigned int Pf = fx->factors[fidx]->P;

    /* initialize the new parameter names. */
    if (fidx == F - 1) {
      for (unsigned int p = 0; p < Pf; p++) {
        /* determine the new string length. */
        const char *facname = OBJECT_TYPE(fd)->name;
        const char *parname = factor_parname(fd, p);
        unsigned int len = strlen(facname);
        len += (parname ? strlen(parname) : 0);
        len += 32;

        /* allocate and build the new string. */
        f->parnames[p0 + p] = malloc(len);
        if (f->parnames[p0 + p])
          snprintf(f->parnames[p0 + p], len - 1, "%s%u.%s",
                   facname, fidx, parname);
      }
    }

    /* increment the parameter offset. */
    p0 += Pf;
  }

  /* return success. */
  return 1;
}

/* product_update(): set the information matrix and parameter vector
 * of a product factor from the values of its underlying child factors.
 *
 * arguments:
 *  @f: factor structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int product_update (factor_t *f) {
  /* get the extended structure pointer. */
  product_t *fx = (product_t*) f;

  /* update the combined information matrix and parameter vector. */
  for (unsigned int n = 0, p0 = 0; n < fx->F; n++) {
    /* get the current factor parameter count. */
    const unsigned int Pf = fx->factors[n]->P;
    if (Pf == 0)
      continue;

    /* get views of the combined matrix and vector. */
    matrix_view_t inf = matrix_submatrix(f->inf, p0, p0, Pf, Pf);
    vector_view_t par = vector_subvector(f->par, p0, Pf);

    /* copy the contents from the factor into the views. */
    matrix_copy(&inf, fx->factors[n]->inf);
    vector_copy(&par, fx->factors[n]->par);

    /* increment the parameter offset. */
    p0 += Pf;
  }

  /* return success. */
  return 1;
}

/* --- */

/* product_getelem(): element getter for product factors.
 *  - see object_getelem_fn() for details.
 */
static object_t *product_getelem (const product_t *obj,
                                  const list_t *idx) {
  /* only admit single-element incides. */
  if (idx->len != 1)
    return NULL;

  /* only admit integer indices. */
  object_t *idxobj = list_get(idx, 0);
  if (!OBJECT_IS_INT(idxobj))
    return NULL;

  /* perform bounds checking on the index. */
  const long idxval = int_get((int_t*) idxobj);
  if (idxval < 0 || (unsigned int) idxval >= obj->F)
    return NULL;

  /* return the product sub-factor. */
  return (object_t*) obj->factors[idxval];
}

/* product_properties: array of accessible object properties.
 */
static object_property_t product_properties[] = {
  FACTOR_PROP_BASE,
  { NULL, NULL, NULL }
};

/* --- */

/* productobj_add(): method for adding factors to product objects.
 *  - see object_method_fn() for details.
 */
static object_t *productobj_add (factor_t *f, map_t *args) {
  /* check for single-factor arguments. */
  object_t *arg = map_get(args, "factor");
  if (arg) {
    /* ensure the argument is a factor. */
    if (!OBJECT_IS_FACTOR(arg))
      return NULL;

    /* add the factor to the product. */
    factor_t *fd = (factor_t*) arg;
    if (!product_add_factor(f, fd->d, fd))
      return NULL;
  }

  /* check for list arguments. */
  arg = map_get(args, "factors");
  if (arg) {
    /* ensure the argument is a list. */
    if (!OBJECT_IS_LIST(arg))
      return NULL;

    /* cast the argument to a list. */
    list_t *lst = (list_t*) arg;

    /* add the list elements to the product. */
    for (size_t i = 0; i < lst->len; i++) {
      /* ensure the list element is a factor. */
      object_t *elem = list_get(lst, i);
      if (!OBJECT_IS_FACTOR(elem))
        return NULL;

      /* add the factor to the product. */
      factor_t *fd = (factor_t*) elem;
      if (!product_add_factor(f, fd->d, fd))
        return NULL;
    }
  }

  /* return nothing. */
  VFL_RETURN_NIL;
}

/* productobj_update(): method for updating product factors.
 *  - see object_method_fn() for details.
 */
static object_t *productobj_update (factor_t *obj, map_t *args) {
  /* update the product factor and return nothing. */
  product_update(obj);
  VFL_RETURN_NIL;
}

/* product_methods: array of callable object methods.
 */
static object_method_t product_methods[] = {
  FACTOR_METHOD_BASE,
  { "add", (object_method_fn) productobj_add },
  { "update", (object_method_fn) productobj_update },
  { NULL, NULL }
};

/* --- */

/* product_type: product factor type structure.
 */
static factor_type_t product_type = {
  { /* base: */
    "product",                                   /* name      */
    sizeof(product_t),                           /* size      */

    (object_init_fn) factor_init,                /* init      */
    (object_copy_fn) factor_copy,                /* copy      */
    (object_free_fn) factor_free,                /* free      */

    (object_binary_fn) factor_add,               /* add       */
    NULL,                                        /* sub       */
    (object_binary_fn) factor_mul,               /* mul       */
    NULL,                                        /* div       */

    (object_getelem_fn) product_getelem,         /* get       */
    NULL,                                        /* set       */
    product_properties,                          /* props     */
    product_methods                              /* methods   */
  },

  1,                                             /* initial D */
  0,                                             /* initial P */
  1,                                             /* initial K */
  NULL,                                          /* parnames  */
  product_eval,                                  /* eval      */
  product_mean,                                  /* mean      */
  product_var,                                   /* var       */
  product_cov,                                   /* cov       */
  product_diff_mean,                             /* diff_mean */
  product_diff_var,                              /* diff_var  */
  product_meanfield,                             /* meanfield */
  product_div,                                   /* div       */
  product_init,                                  /* init      */
  product_resize,                                /* resize    */
  product_kernel,                                /* kernel    */
  product_set,                                   /* set       */
  product_copy,                                  /* copy      */
  product_free                                   /* free      */
};

/* vfl_factor_product: address of the product_type
 * structure.
 */
const factor_type_t *vfl_factor_product = &product_type;

