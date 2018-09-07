
/* include the vfl header. */
#include <vfl/vfl.h>
#include <vfl/factor/product.h>

/* define documentation strings: */

PyDoc_STRVAR(
  Product_doc,
"Product() -> Product object\n"
"\n");

PyDoc_STRVAR(
  Product_method_update_doc,
"Update product factor parameters to match its member factors.\n"
"\n");

/* product_new_with_size(): allocate a new product factor with an array
 * of a certain size, ready to be populated.
 *
 * arguments:
 *  @F: size of the new product factor's array.
 *  @D, @P, @K: sizes to pass to factor_resize().
 *
 * returns:
 *  new product factor, or NULL on failure.
 */
PyObject *product_new_with_size (size_t F, size_t D, size_t P, size_t K) {
  /* allocate a new factor array. */
  Factor **factors = malloc(F * sizeof(Factor*));
  if (!factors) {
    PyErr_SetNone(PyExc_MemoryError);
    return NULL;
  }

  /* create a new product factor. */
  PyObject *self = PyObject_CallObject((PyObject*) &Product_Type, NULL);
  if (!self) {
    free(factors);
    return NULL;
  }

  /* resize the product factor. */
  if (!factor_resize((Factor*) self, D, P, K)) {
    Py_DECREF(self);
    free(factors);
    return NULL;
  }

  /* get the extended structure pointer. */
  Product *fx = (Product*) self;

  /* initialize the array contents. */
  for (size_t i = 0; i < F; i++)
    factors[i] = NULL;

  /* store the factor array and count. */
  fx->factors = factors;
  fx->F = F;

  /* return the new factor. */
  return self;
}

/* product_get_size(): get the size of a product's factor array.
 *
 * arguments:
 *  @self: object structure pointer.
 *
 * returns:
 *  size of the factor array.
 */
size_t product_get_size (PyObject *self) {
  /* return the factor count of the product factor. */
  return (Product_Check(self) ? Product_GET_SIZE(self) : 0);
}

/* product_set_item(): store an item into a product's factor array.
 *
 * arguments:
 *  @self: object structure pointer.
 *  @i: index in the factor array.
 *  @v: value to store into the array.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int product_set_item (PyObject *self, size_t i, PyObject *v) {
  /* check that the object is a product. */
  if (!self || !Product_Check(self))
    return 0;

  /* check that the value is a factor. */
  if (!v || !Factor_Check(v))
    return 0;

  /* check that the index is in bounds. */
  if (i >= Product_GET_SIZE(self))
    return 0;

  /* set the object and return success. */
  Product_SET_ITEM(self, i, v);
  return 1;
}

/* product_update(): set the information matrix and parameter vector
 * of a product factor from the values of its underlying child factors.
 *
 * arguments:
 *  @self: object structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int product_update (PyObject *self) {
  /* check that the object is a product. */
  if (!Product_Check(self))
    return 0;

  /* get the extended structure pointers. */
  Product *fx = (Product*) self;
  Factor *f = (Factor*) self;

  /* update the combined information matrix and parameter vector. */
  for (size_t n = 0, p0 = 0; n < fx->F; n++) {
    /* get the current factor parameter count. */
    const size_t Pf = fx->factors[n]->P;
    if (Pf == 0)
      continue;

    /* get views of the combined matrix and vector. */
    MatrixView inf = matrix_submatrix(f->inf, p0, p0, Pf, Pf);
    VectorView par = vector_subvector(f->par, p0, Pf);

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

/* Product_eval(): evaluate the product factor at its mode.
 *  - see factor_mean_fn() for more information.
 */
FACTOR_EVAL (Product) {
  /* get the extended structure pointer. */
  Product *fx = (Product*) f;

  /* include the values of each factor. */
  double mode = 1.0;
  for (size_t n = 0; n < fx->F; n++) {
    Factor *fn = fx->factors[n];
    mode *= factor_eval(fn, x, p, i % fn->K);
  }

  /* return the computed mode. */
  return mode;
}

/* Product_mean(): evaluate the product factor mean.
 *  - see factor_mean_fn() for more information.
 */
FACTOR_MEAN (Product) {
  /* get the extended structure pointer. */
  Product *fx = (Product*) f;

  /* include the means of each factor. */
  double mean = 1.0;
  for (size_t n = 0; n < fx->F; n++) {
    Factor *fn = fx->factors[n];
    mean *= factor_mean(fn, x, p, i % fn->K);
  }

  /* return the computed expectation. */
  return mean;
}

/* Product_var(): evaluate the product factor variance.
 *  - see factor_var_fn() for more information.
 */
FACTOR_VAR (Product) {
  /* get the extended structure pointer. */
  Product *fx = (Product*) f;

  /* include the variances of each factor. */
  double var = 1.0;
  for (size_t n = 0; n < fx->F; n++) {
    Factor *fn = fx->factors[n];
    var *= factor_var(fn, x, p, i % fn->K, j % fn->K);
  }

  /* return the computed expectation. */
  return var;
}

/* Product_cov(): evaluate the product factor covariance.
 *  - see factor_cov_fn() for more information.
 */
FACTOR_COV (Product) {
  /* get the extended structure pointer. */
  Product *fx = (Product*) f;

  /* include the covariances of each factor. */
  double cov = 1.0;
  for (size_t n = 0; n < fx->F; n++) {
    Factor *fn = fx->factors[n];
    cov *= factor_cov(fn, x1, x2, p1, p2);
  }

  /* return the computed expectation. */
  return cov;
}

/* Product_diff_mean(): evaluate the product factor mean gradient.
 *  - see factor_diff_mean_fn() for more information.
 */
FACTOR_DIFF_MEAN (Product) {
  /* get the extended structure pointer and factor count. */
  Product *fx = (Product*) f;
  const size_t F = fx->F;

  /* initialize the output vector with each factor gradient. */
  for (size_t n2 = 0, p0 = 0; n2 < F; n2++) {
    /* get the current factor parameter count. */
    Factor *f2 = fx->factors[n2];
    const size_t Pf = f2->P;

    /* store the factor gradient into the appropriate subvector. */
    VectorView df2 = vector_subvector(df, p0, Pf);
    factor_diff_mean(f2, x, p, i % f2->K, &df2);

    /* increment the parameter offset. */
    p0 += Pf;
  }

  /* loop to include "other-factor" means in the output. */
  for (size_t n1 = 0; n1 < F; n1++) {
    /* compute the current factor mean. */
    Factor *f1 = fx->factors[n1];
    const double mean1 = factor_mean(f1, x, p, i % f1->K);

    /* loop over the factors again. */
    for (size_t n2 = 0, p0 = 0; n2 < F; n2++) {
      /* get the current factor parameter count. */
      const size_t Pf = fx->factors[n2]->P;

      /* include only "other-factor" contributions. */
      if (n2 != n1) {
        VectorView df2 = vector_subvector(df, p0, Pf);
        blas_dscal(mean1, &df2);
      }

      /* increment the parameter offset. */
      p0 += Pf;
    }
  }
}

/* Product_diff_var(): evaluate the product factor variance gradient.
 *  - see factor_diff_var_fn() for more information.
 */
FACTOR_DIFF_VAR (Product) {
  /* get the extended structure pointer and factor count. */
  Product *fx = (Product*) f;
  const size_t F = fx->F;

  /* initialize the output vector with each factor gradient. */
  for (size_t n2 = 0, p0 = 0; n2 < F; n2++) {
    /* get the current factor parameter count. */
    Factor *f2 = fx->factors[n2];
    const size_t Pf = f2->P;

    /* store the factor gradient into the appropriate subvector. */
    VectorView df2 = vector_subvector(df, p0, Pf);
    factor_diff_var(f2, x, p, i % f2->K, j % f2->K, &df2);

    /* increment the parameter offset. */
    p0 += Pf;
  }

  /* loop to include "other-factor" variances in the output. */
  for (size_t n1 = 0; n1 < F; n1++) {
    /* compute the current factor variance. */
    Factor *f1 = fx->factors[n1];
    const double var1 = factor_var(f1, x, p, i % f1->K, j % f1->K);

    /* loop over the factors again. */
    for (size_t n2 = 0, p0 = 0; n2 < F; n2++) {
      /* get the current factor parameter count. */
      const size_t Pf = fx->factors[n2]->P;

      /* include only "other-factor" contributions. */
      if (n2 != n1) {
        VectorView df2 = vector_subvector(df, p0, Pf);
        blas_dscal(var1, &df2);
      }

      /* increment the parameter offset. */
      p0 += Pf;
    }
  }
}

/* Product_meanfield(): perform a mean-field update of a product factor.
 *  - see factor_meanfield_fn() for more information.
 */
FACTOR_MEANFIELD (Product) {
  /* get the extended structure pointers. */
  Product *fx = (Product*) f;
  Product *fpx = (Product*) fp;

  /* check for initialization calls. */
  if (FACTOR_MEANFIELD_INIT) {
    /* initialize all sub-factors. */
    int ret = 1;
    for (size_t n = 0; n < fx->F; n++)
      ret &= factor_meanfield(fx->factors[n], NULL, NULL, NULL, NULL);

    /* return the result of the initializations. */
    return ret;
  }

  /* check for finalization calls. */
  if (FACTOR_MEANFIELD_END) {
    /* finalize all sub-factors. */
    int ret = 1;
    for (size_t n = 0; n < fx->F; n++)
      ret &= factor_meanfield(fx->factors[n], fpx->factors[n],
                              NULL, NULL, NULL);

    /* update the parameter vector and information matrix. */
    if (ret) product_update((PyObject*) f);

    /* return the result of the finalizations. */
    return ret;
  }

  /* get the sub-factor coefficients. */
  Vector *bn = fx->b0;
  Matrix *Bn = fx->B0;

  /* loop over the sub-factors. */
  int ret = 1;
  for (size_t n = 0; n < fx->F; n++) {
    /* get the current sub-factor and sub-prior. */
    Factor *fn = fx->factors[n];
    Factor *fpn = fpx->factors[n];

    /* initialize the coefficients. */
    vector_copy(bn, b);
    matrix_copy(Bn, B);

    /* include the expectations from the other sub-factors. */
    for (size_t n2 = 0; n2 < fx->F; n2++) {
      /* skip the current sub-factor. */
      if (n2 == n) continue;

      /* adjust the coefficient vector. */
      const Factor *fn2 = fx->factors[n2];
      for (size_t k = 0; k < f->K; k++) {
        const double phi1 = factor_mean(fn2, dat->x, dat->p, k % fn2->K);
        vector_set(bn, k, vector_get(bn, k) * phi1);
      }

      /* adjust the coefficient matrix. */
      for (size_t k = 0; k < f->K; k++) {
        for (size_t k2 = 0; k2 < f->K; k2++) {
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

/* Product_div(): evaluate the product factor divergence.
 *  - see factor_div_fn() for more information.
 */
FACTOR_DIV (Product) {
  /* get the extended structure pointers. */
  Product *fx = (Product*) f;
  Product *f2x = (Product*) f2;

  /* sum the divergences of each factor together. */
  double div = 0.0;
  for (size_t n = 0; n < fx->F; n++)
    div += factor_div(fx->factors[n], f2x->factors[n]);

  /* return the computed divergence. */
  return div;
}

/* Product_resize(): handle resizes of the product factor.
 *  - see factor_resize_fn() for more information.
 */
FACTOR_RESIZE (Product) {
  /* get the extended structure pointer. */
  Product *fx = (Product*) f;

  /* allocate new mean-field variables. */
  Vector *b0 = vector_alloc(K);
  Matrix *B0 = matrix_alloc(K, K);
  if (!b0 || !B0) {
    vector_free(b0);
    matrix_free(B0);
    return 0;
  }

  /* free the mean-field variables. */
  vector_free(fx->b0);
  matrix_free(fx->B0);

  /* store the new variables. */
  fx->b0 = b0;
  fx->B0 = B0;

  /* return success. */
  return 1;
}

/* Product_kernel(): write the kernel code of a product factor.
 *  - see factor_kernel_fn() for more information.
 */
FACTOR_KERNEL (Product) {
  /* get the extended structure pointer. */
  Product *fx = (Product*) f;

  /* define kernel code format strings. */
  const char *fmtA = "double prod = 1.0;\n";
  const char *fmtB = "{\n%s}\nprod *= cov;\n";
  const char *fmtC = "cov = prod;\n";

  /* allocate an array for storing sub-factor kernel code. */
  char **fstr = malloc(fx->F * sizeof(char*));
  if (!fstr)
    return NULL;

  /* get the strings of each sub-factor. */
  for (size_t n = 0, pn = p0; n < fx->F; n++) {
    /* get the current sub-factor string. */
    const Factor *fn = fx->factors[n];
    fstr[n] = factor_kernel(fn, pn);

    /* check for failure. */
    if (!fstr[n])
      return NULL;

    /* advance the sub-factor parameter offset. */
    pn += fn->P;
  }

  /* determine the length of the kernel code string. */
  size_t len = strlen(fmtA) + strlen(fmtC) + 8;
  for (size_t n = 0; n < fx->F; n++)
    len += strlen(fmtB) + strlen(fstr[n]);

  /* allocate the kernel code string. */
  char *kstr = malloc(len);
  if (!kstr)
    return NULL;

  /* write the header. */
  char *pos = kstr;
  pos += sprintf(pos, "%s", fmtA);

  /* write each sub-factor string. */
  for (size_t n = 0; n < fx->F; n++)
    pos += sprintf(pos, fmtB, fstr[n]);

  /* write the footer. */
  sprintf(pos, "%s", fmtC);

  /* free the sub-factor strings. */
  for (size_t n = 0; n < fx->F; n++)
    free(fstr[n]);

  /* free the sub-factor string array. */
  free(fstr);

  /* return the new string. */
  return kstr;
}

/* Product_set(): store a parameter into a product factor.
 *  - see factor_set_fn() for more information.
 */
FACTOR_SET (Product) {
  /* get the extended structure pointer. */
  Product *fx = (Product*) f;

  /* loop over the factor array. */
  for (size_t n = 0, p0 = 0; n < fx->F; n++) {
    /* get the current factor. */
    Factor *fn = fx->factors[n];
    const size_t Pn = fn->P;

    /* if the parameter index is now in bounds, set the parameter
     * of the current factor, using the parameter array offset.
     */
    if (i < p0 + Pn) {
      /* attempt to set the factor parameter. */
      if (!factor_set(fn, i - p0, value))
        return 0;

      /* get views of the combined vector and matrix. */
      MatrixView inf = matrix_submatrix(f->inf, p0, p0, Pn, Pn);
      VectorView par = vector_subvector(f->par, p0, Pn);

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

/* Product_copy(): copy extra information between product factors.
 *  - see factor_copy_fn() for more information.
 */
FACTOR_COPY (Product) {
  /* get the extended structure pointers. */
  Product *fdupx = (Product*) fdup;
  Product *fx = (Product*) f;

  /* copy the factor count. */
  const size_t F = fx->F;
  fdupx->F = F;

  /* allocate the duplicate factor array. */
  fdupx->factors = malloc(F * sizeof(Factor*));
  if (!fdupx->factors)
    return 0;

  /* initialize the duplicate factor array. */
  for (size_t i = 0; i < F; i++)
    fdupx->factors[i] = NULL;

  /* copy each factor into the duplicate factor array. */
  for (size_t i = 0; i < F; i++) {
    fdupx->factors[i] = factor_copy(fx->factors[i]);
    Py_XINCREF(fdupx->factors[i]);
    if (!fdupx->factors[i])
      return 0;
  }

  /* return success. */
  return 1;
}

/* Product_free(): free extra information from product factors.
 *  - see factor_free_fn() for more information.
 */
FACTOR_FREE (Product) {
  /* get the extended structure pointer. */
  Product *fx = (Product*) f;

  /* release our reference to each sub-factor. */
  for (size_t i = 0; i < fx->F; i++)
    Py_XDECREF(fx->factors[i]);

  /* free the array of factors. */
  free(fx->factors);

  /* free the mean-field variables. */
  vector_free(fx->b0);
  matrix_free(fx->B0);
}

/* --- */

/* Product_seq_len(): method for getting product sub-factor counts.
 */
static Py_ssize_t
Product_seq_len (Product *self) {
  /* return the current size of the factors array. */
  return (Py_ssize_t) self->F;
}

/* Product_seq_get(): method for getting product sub-factors.
 */
static PyObject*
Product_seq_get (Product *self, Py_ssize_t i) {
  /* check that the index is in bounds. */
  const Py_ssize_t n = Product_seq_len(self);
  if (n == 0 || i < 0 || i >= n) {
    PyErr_SetNone(PyExc_IndexError);
    return NULL;
  }

  /* return a new reference to the indexed factor. */
  Py_INCREF(self->factors[i]);
  return (PyObject*) self->factors[i];
}

/* --- */

/* Product_method_update(): update the internals of a product factor.
 */
static PyObject*
Product_method_update (PyObject *self, PyObject *args) {
  /* call the product factor update function. */
  product_update(self);
  Py_RETURN_NONE;
}

/* --- */

/* Product_new(): allocate a new product factor.
 *  - see PyTypeObject.tp_new for details.
 */
VFL_TYPE_NEW (Product) {
  /* allocate a new product factor. */
  Product *self = (Product*) type->tp_alloc(type, 0);
  Factor_reset((Factor*) self);
  if (!self)
    return NULL;

  /* initialize the factors array. */
  self->factors = NULL;
  self->F = 0;

  /* initialize the mean-field variables. */
  self->b0 = NULL;
  self->B0 = NULL;

  /* initialize the function pointers. */
  Factor *f = (Factor*) self;
  f->eval      = Product_eval;
  f->mean      = Product_mean;
  f->var       = Product_var;
  f->cov       = Product_cov;
  f->diff_mean = Product_diff_mean;
  f->diff_var  = Product_diff_var;
  f->meanfield = Product_meanfield;
  f->div       = Product_div;
  f->resize    = Product_resize;
  f->kernel    = Product_kernel;
  f->set       = Product_set;
  f->copy      = Product_copy;
  f->free      = Product_free;

  /* resize to the default size. */
  if (!factor_resize(f, 1, 0, 1)) {
    Py_DECREF(f);
    return NULL;
  }

  /* return the new object. */
  return (PyObject*) self;
}

/* Product_sequence: sequence definition structure for product factors.
 */
static PySequenceMethods Product_sequence = {
  (lenfunc) Product_seq_len,                     /* sq_length         */
  NULL,                                          /* sq_concat         */
  NULL,                                          /* sq_repeat         */
  (ssizeargfunc) Product_seq_get,                /* sq_item           */
  NULL,
  NULL,                                          /* sq_ass_item       */
  NULL,
  NULL,                                          /* sq_contains       */
  NULL,                                          /* sq_inplace_concat */
  NULL                                           /* sq_inplace_repeat */
};

/* Product_getset: property definition structure for product factors.
 */
static PyGetSetDef Product_getset[] = {
  { NULL }
};

/* Product_methods: method definition structure for product factors.
 */
static PyMethodDef Product_methods[] = {
  { "update",
    (PyCFunction) Product_method_update,
    METH_VARARGS,
    Product_method_update_doc
  },
  { NULL }
};

/* Product_Type: type definition structure for product factors.
 */
PyTypeObject Product_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "factor.Product",                              /* tp_name           */
  sizeof(Product),                               /* tp_basicsize      */
  0,                                             /* tp_itemsize       */
  0,                                             /* tp_dealloc        */
  0,                                             /* tp_print          */
  0,                                             /* tp_getattr        */
  0,                                             /* tp_setattr        */
  0,                                             /* tp_reserved       */
  0,                                             /* tp_repr           */
  0,                                             /* tp_as_number      */
  &Product_sequence,                             /* tp_as_sequence    */
  0,                                             /* tp_as_mapping     */
  0,                                             /* tp_hash           */
  0,                                             /* tp_call           */
  0,                                             /* tp_str            */
  0,                                             /* tp_getattro       */
  0,                                             /* tp_setattro       */
  0,                                             /* tp_as_buffer      */
  Py_TPFLAGS_DEFAULT,                            /* tp_flags          */
  Product_doc,                                   /* tp_doc            */
  0,                                             /* tp_traverse       */
  0,                                             /* tp_clear          */
  0,                                             /* tp_richcompare    */
  0,                                             /* tp_weaklistoffset */
  0,                                             /* tp_iter           */
  0,                                             /* tp_iternext       */
  Product_methods,                               /* tp_methods        */
  0,                                             /* tp_members        */
  Product_getset,                                /* tp_getset         */
  &Factor_Type,                                  /* tp_base           */
  0,                                             /* tp_dict           */
  0,                                             /* tp_descr_get      */
  0,                                             /* tp_descr_set      */
  0,                                             /* tp_dictoffset     */
  0,                                             /* tp_init           */
  0,                                             /* tp_alloc          */
  Product_new                                    /* tp_new            */
};

/* Product_Type_init() */
VFL_TYPE_INIT (Product)

