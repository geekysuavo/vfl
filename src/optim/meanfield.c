
/* include the vfl header. */
#include <vfl/vfl.h>

/* MeanField: structure for holding mean-field optimizers.
 */
typedef struct {
  /* optimizer superclass. */
  Optim super;

  /* subclass struct members. */
}
MeanField;

/* define documentation strings: */

PyDoc_STRVAR(
  MeanField_doc,
"MeanField() -> MeanField object\n"
"\n");

/* MeanField_iterate(): iteration function for MeanField.
 *  - see optim_iterate_fn() for more information.
 */
OPTIM_ITERATE (MeanField) {
  /* declare variables to track the bound between factor updates. */
  double bound, bound_init;

  /* initialize the model using a full inference. */
  model_infer(opt->mdl);
  bound = bound_init = model_bound(opt->mdl);

  /* determine the maximum number of factors to update. */
  size_t M = 0, K = 0;
  const size_t Kmax = opt->mdl->dat->N;
  for (size_t j = 0; j < opt->mdl->M; j++) {
    const size_t Kj = opt->mdl->factors[j]->K;
    if (K + Kj < Kmax) {
      K += Kj;
      M++;
    }
    else
      break;
  }

  /* update each factor in the model. */
  for (size_t j = 0; j < M; j++) {
    /* update the factor and, if necessary, re-infer the weights. */
    if (model_meanfield(opt->mdl, j))
      model_update(opt->mdl, j);

    /* recalculate the bound. */
    bound = model_bound(opt->mdl);
  }

  /* store the new lower bound into the optimizer. */
  opt->bound = bound;

  /* return whether or not the bound was changed by iteration. */
  return (bound != bound_init);
}

/* MeanField_execute(): execution function for MeanField.
 *  - see optim_iterate_fn() for more information.
 */
OPTIM_EXECUTE (MeanField) {
  /* store the lower bound from the previous iteration. */
  double bound_prev = opt->bound;

  /* loop for the specified number of iterations. */
  for (size_t iter = 0; iter < opt->max_iters; iter++) {
    /* perform an iteration, and break if the bound does not improve. */
    bound_prev = opt->bound;
    if (optim_iterate(opt) || opt->bound < bound_prev)
      break;
  }

  /* return whether the bound was changed by the final iteration. */
  return (opt->bound > bound_prev);
}

/* --- */

/* MeanField_new(): allocate a new mean-field optimizer.
 *  - see PyTypeObject.tp_new for details.
 */
VFL_TYPE_NEW (MeanField) {
  /* allocate a new mean-field optimizer. */
  MeanField *self = (MeanField*) type->tp_alloc(type, 0);
  Optim_reset((Optim*) self);
  if (!self)
    return NULL;

  /* set the function pointers. */
  Optim *opt = (Optim*) self;
  opt->iterate = MeanField_iterate;
  opt->execute = MeanField_execute;

  /* return the new object. */
  return (PyObject*) self;
}

/* MeanField_getset: property definition structure for
 * mean-field optimizers.
 */
static PyGetSetDef MeanField_getset[] = {
  { NULL }
};

/* MeanField_methods: method definition structure for
 * mean-field optimizers.
 */
static PyMethodDef MeanField_methods[] = {
  { NULL }
};

/* MeanField_Type, MeanField_Type_init() */
VFL_TYPE (MeanField, Optim, optim)

