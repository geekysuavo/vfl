
/* include the model header. */
#include <vfl/model.h>

/* include the required object headers. */
#include <vfl/base/map.h>
#include <vfl/base/list.h>
#include <vfl/base/float.h>

/* model_init(): initialize a variational feature model.
 *
 * arguments:
 *  @mdl: model structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int model_init (model_t *mdl) {
  /* get the model type information. */
  const model_type_t *type = MODEL_TYPE(mdl);

  /* initialize the sizes of the model. */
  mdl->D = 0;
  mdl->P = 0;
  mdl->M = 0;
  mdl->K = 0;

  /* initialize the prior parameters. */
  mdl->alpha0 = 1.0;
  mdl->beta0 = 1.0;
  mdl->nu = 1.0;

  /* initialize the posterior noise parameters. */
  mdl->alpha = mdl->alpha0;
  mdl->beta = mdl->beta0;
  mdl->tau = mdl->alpha / mdl->beta;

  /* initialize the posterior weight parameters. */
  mdl->wbar = NULL;
  mdl->Sigma = NULL;

  /* initialize the posterior logistic parameters. */
  mdl->xi = NULL;

  /* initialize the intermediates. */
  mdl->Sinv = NULL;
  mdl->L = NULL;
  mdl->h = NULL;

  /* initialize the prior and posterior factor arrays. */
  mdl->factors = NULL;
  mdl->priors = NULL;

  /* initialize the associated dataset. */
  mdl->dat = NULL;

  /* initialize the temporary vector. */
  mdl->tmp = NULL;

  /* execute the initialization function, if defined. */
  model_init_fn init_fn = type->init;
  if (init_fn && !init_fn(mdl))
    return 0;

  /* return success. */
  return 1;
}

/* model_free(): free the contents of a variational feature model.
 *
 * arguments:
 *  @mdl: model structure pointer to free.
 */
void model_free (model_t *mdl) {
  /* free the weight means and covariances. */
  vector_free(mdl->wbar);
  matrix_free(mdl->Sigma);

  /* free the logistic parameters. */
  vector_free(mdl->xi);

  /* free the intermediates. */
  matrix_free(mdl->Sinv);
  matrix_free(mdl->L);
  vector_free(mdl->h);

  /* free the individual prior and posterior factors. */
  for (unsigned int i = 0; i < mdl->M; i++) {
    obj_free((object_t*) mdl->factors[i]);
    obj_free((object_t*) mdl->priors[i]);
  }

  /* free the factor arrays. */
  free(mdl->factors);
  free(mdl->priors);

  /* free the temporary vector. */
  vector_free(mdl->tmp);
}

/* --- */

/* model_getprop_dims(): method for getting model dimension counts.
 *  - see object_getprop_fn() for details.
 */
object_t *model_getprop_dims (const model_t *mdl) {
  /* return the dimension count as an integer. */
  return (object_t*) int_alloc_with_value(mdl->D);
}

/* model_getprop_pars(): method for getting model parameter counts.
 *  - see object_getprop_fn() for details.
 */
object_t *model_getprop_pars (const model_t *mdl) {
  /* return the parameter count as an integer. */
  return (object_t*) int_alloc_with_value(mdl->P);
}

/* model_getprop_cmps(): method for getting model factor counts.
 *  - see object_getprop_fn() for details.
 */
object_t *model_getprop_cmps (const model_t *mdl) {
  /* return the component/factor count as an integer. */
  return (object_t*) int_alloc_with_value(mdl->M);
}

/* model_getprop_wgts(): method for getting model weight counts.
 *  - see object_getprop_fn() for details.
 */
object_t *model_getprop_wgts (const model_t *mdl) {
  /* return the weight/coefficient count as an integer. */
  return (object_t*) int_alloc_with_value(mdl->K);
}

/* model_getprop_data(): method for getting model datasets.
 *  - see object_getprop_fn() for details.
 */
data_t *model_getprop_data (const model_t *mdl) {
  /* return the dataset from the model. */
  return mdl->dat;
}

/* model_getprop_factors(): method for getting model factor lists.
 *  - see object_getprop_fn() for details.
 */
object_t *model_getprop_factors (const model_t *mdl) {
  /* return nothing if the model has no factors. */
  if (mdl->M == 0)
    VFL_RETURN_NIL;

  /* allocate a new list to hold the factors. */
  list_t *lst = list_alloc_with_length(mdl->M);
  if (!lst)
    return NULL;

  /* store the factors in the list. */
  for (unsigned int j = 0; j < mdl->M; j++)
    list_set(lst, j, (object_t*) mdl->factors[j]);

  /* return the new list of factors. */
  return (object_t*) lst;
}

/* model_getprop_alpha0(): method for getting prior noise shape.
 *  - object_getprop_fn() for details.
 */
object_t *model_getprop_alpha0 (const model_t *mdl) {
  /* return the property value as a float. */
  return (object_t*) float_alloc_with_value(mdl->alpha0);
}

/* model_getprop_beta0(): method for getting prior noise rate.
 *  - object_getprop_fn() for details.
 */
object_t *model_getprop_beta0 (const model_t *mdl) {
  /* return the property value as a float. */
  return (object_t*) float_alloc_with_value(mdl->beta0);
}

/* model_getprop_alpha(): method for getting posterior noise shape.
 *  - object_getprop_fn() for details.
 */
object_t *model_getprop_alpha (const model_t *mdl) {
  /* return the property value as a float. */
  return (object_t*) float_alloc_with_value(mdl->alpha);
}

/* model_getprop_beta(): method for getting posterior noise rate.
 *  - object_getprop_fn() for details.
 */
object_t *model_getprop_beta (const model_t *mdl) {
  /* return the property value as a float. */
  return (object_t*) float_alloc_with_value(mdl->beta);
}

/* model_getprop_tau(): method for getting posterior noise precision.
 *  - object_getprop_fn() for details.
 */
object_t *model_getprop_tau (const model_t *mdl) {
  /* return the property value as a float. */
  return (object_t*) float_alloc_with_value(mdl->tau);
}

/* model_getprop_nu(): method for getting prior noise/weight ratio.
 *  - object_getprop_fn() for details.
 */
object_t *model_getprop_nu (const model_t *mdl) {
  /* return the property value as a float. */
  return (object_t*) float_alloc_with_value(mdl->nu);
}

/* --- */

/* model_setprop_data(): method for setting model datasets.
 *  - see object_setprop_fn() for details.
 */
int model_setprop_data (model_t *mdl, object_t *val) {
  /* ensure the value is a dataset. */
  if (!OBJECT_IS_DATA(val))
    return 0;

  /* store the dataset in the model. */
  return model_set_data(mdl, (data_t*) val);
}

/* model_setprop_factors(): method for setting model factor lists.
 *  - see object_setprop_fn() for details.
 */
int model_setprop_factors (model_t *mdl, object_t *val) {
  /* check the value type. */
  if (OBJECT_IS_FACTOR(val)) {
    /* add the factor to the model. */
    if (!model_add_factor(mdl, (factor_t*) val))
      return 0;
  }
  else if (OBJECT_IS_LIST(val)) {
    /* add the list elements to the model. */
    list_t *lst = (list_t*) val;
    for (size_t i = 0; i < lst->len; i++) {
      /* ensure the element is a factor. */
      object_t *elem = list_get(lst, i);
      if (!OBJECT_IS_FACTOR(elem))
        return 0;

      /* add the factor to the model. */
      if (!model_add_factor(mdl, (factor_t*) elem))
        return 0;
    }
  }
  else
    return 0;

  /* return success. */
  return 1;
}

/* model_setprop_alpha0(): method for setting prior noise shape.
 *  - object_setprop_fn() for details.
 */
int model_setprop_alpha0 (model_t *mdl, object_t *val) {
  /* admit only integer and float values. */
  if (OBJECT_IS_NUM(val)) return model_set_alpha0(mdl, num_get(val));
  return 0;
}

/* model_setprop_beta0(): method for setting prior noise rate.
 *  - object_setprop_fn() for details.
 */
int model_setprop_beta0 (model_t *mdl, object_t *val) {
  /* admit only integer and float values. */
  if (OBJECT_IS_NUM(val)) return model_set_beta0(mdl, num_get(val));
  return 0;
}

/* model_setprop_tau(): method for getting posterior noise precision.
 *  - object_setprop_fn() for details.
 */
int model_setprop_tau (model_t *mdl, object_t *val) {
  /* admit only integer and float values. */
  if (OBJECT_IS_NUM(val)) return tauvfr_set_tau(mdl, num_get(val));
  return 0;
}

/* model_setprop_nu(): method for setting prior noise/weight ratio.
 *  - object_setprop_fn() for details.
 */
int model_setprop_nu (model_t *mdl, object_t *val) {
  /* admit only integer and float values. */
  if (OBJECT_IS_NUM(val)) return model_set_nu(mdl, num_get(val));
  return 0;
}

/* --- */

/* model_method_add(): method for adding factors to models.
 *  - see object_method_fn() for details.
 */
object_t *model_method_add (model_t *mdl, object_t *args) {
  /* check for the single-factor argument. */
  object_t *arg = map_get((map_t*) args, "factor");
  if (arg && !model_setprop_factors(mdl, arg))
    return NULL;

  /* check for the multi-factor argument. */
  arg = map_get((map_t*) args, "factors");
  if (arg && !model_setprop_factors(mdl, arg))
    return NULL;

  /* return nothing. */
  VFL_RETURN_NIL;
}

/* model_method_infer(): method for executing model inference.
 *  - see object_method_fn() for details.
 */
object_t *model_method_reset (model_t *mdl, object_t *args) {
  /* execute the model reset function and return nothing. */
  model_reset(mdl);
  VFL_RETURN_NIL;
}

/* model_method_reset(): method for resetting models to prior values.
 *  - see object_method_fn() for details.
 */
object_t *model_method_infer (model_t *mdl, object_t *args) {
  /* execute the model inference function and return nothing. */
  model_infer(mdl);
  VFL_RETURN_NIL;
}

/* model_method_eval(): method for evaluating models.
 *  - see object_method_fn() for details.
 */
object_t *model_method_eval (model_t *mdl, object_t *args) {
  /* check for dataset arguments. */
  object_t *dat = map_get((map_t*) args, "data");
  if (dat) {
    /* check the argument type. */
    if (!OBJECT_IS_DATA(dat))
      return NULL;

    /* perform dataset-based evaluation. */
    if (!model_eval_all(mdl, (data_t*) dat))
      return NULL;

    /* return nothing. */
    VFL_RETURN_NIL;
  }

  /* FIXME: check for value arguments. */

  /* invalid arguments, return failure. */
  return NULL;
}

/* model_method_predict(): method for computing model predictions.
 *  - see object_method_fn() for details.
 */
object_t *model_method_predict (model_t *mdl, object_t *args) {
  /* check for the dataset arguments. */
  object_t *mean = map_get((map_t*) args, "mean");
  object_t *var  = map_get((map_t*) args, "var");
  if (mean || var) {
    /* check the argument types. */
    if (mean && !OBJECT_IS_DATA(mean)) return NULL;
    if (var  && !OBJECT_IS_DATA(var))  return NULL;

    /* perform dataset-based prediction. */
    if (!model_predict_all(mdl, (data_t*) mean, (data_t*) var))
      return NULL;

    /* return nothing. */
    VFL_RETURN_NIL;
  }

  /* FIXME: check for value arguments. */

  /* invalid arguments, return failure. */
  return NULL;
}

