
/* include the vfl header. */
#include <vfl/vfl.h>

/* model_types: central registry of model type pointers.
 */
static model_type_t **model_types;
static unsigned int n_model_types;

/* optim_types: central registry of optimizer type pointers.
 */
static optim_type_t **optim_types;
static unsigned n_optim_types;

/* factor_types: central registry of factor type pointers.
 */
static factor_type_t **factor_types;
static unsigned int n_factor_types;

/* vfl_init(): initialize the central registries of object types,
 * and register the core set of models, optimizers, and factors.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int vfl_init (void) {
  /* declare required variables:
   *  @res: combined result of registering core vfl types.
   */
  int res = 1;

  /* initialize the model type registry. */
  model_types = NULL;
  n_model_types = 0;

  /* initialize the optimizer type registry. */
  optim_types = NULL;
  n_optim_types = 0;

  /* initialize the factor type registry. */
  factor_types = NULL;
  n_factor_types = 0;

  /* register core model types. */
  res &= vfl_register_model_type(model_type_vfc);
  res &= vfl_register_model_type(model_type_vfr);
  res &= vfl_register_model_type(model_type_tauvfr);

  /* register core optimizer types. */
  res &= vfl_register_optim_type(optim_type_fg);

  /* register core factor types. */
  res &= vfl_register_factor_type(factor_type_cosine);
  res &= vfl_register_factor_type(factor_type_decay);
  res &= vfl_register_factor_type(factor_type_fixed_impulse);
  res &= vfl_register_factor_type(factor_type_impulse);
  res &= vfl_register_factor_type(factor_type_polynomial);
  res &= vfl_register_factor_type(factor_type_product);

  /* return the result. */
  return res;
}

/* vfl_register_model_type(): store a new model type in the
 * central registry of model types.
 *
 * arguments:
 *  @type: model type structure pointer.
 *
 * returns:
 *  integer indicating registration success (1) or failure (0).
 */
int vfl_register_model_type (const model_type_t *type) {
  /* check the type structure pointer. */
  if (!type)
    return 0;

  /* resize the model type array. */
  n_model_types++;
  const unsigned int bytes = n_model_types * sizeof(model_type_t*);
  model_types = (model_type_t**) realloc(model_types, bytes);
  if (!model_types)
    return 0;

  /* store the new type and return success. */
  model_types[n_model_types - 1] = (model_type_t*) type;
  return 1;
}

/* vfl_register_optim_type(): store a new optimizer type in the
 * central registry of optimizer types.
 *
 * arguments:
 *  @type: optimizer type structure pointer.
 *
 * returns:
 *  integer indicating registration success (1) or failure (0).
 */
int vfl_register_optim_type (const optim_type_t *type) {
  /* check the type structure pointer. */
  if (!type)
    return 0;

  /* resize the optimizer type array. */
  n_optim_types++;
  const unsigned int bytes = n_optim_types * sizeof(optim_type_t*);
  optim_types = (optim_type_t**) realloc(optim_types, bytes);
  if (!optim_types)
    return 0;

  /* store the new type and return success. */
  optim_types[n_optim_types - 1] = (optim_type_t*) type;
  return 1;
}

/* vfl_register_factor_type(): store a new factor type in the
 * central registry of factor types.
 *
 * arguments:
 *  @type: factor type structure pointer.
 *
 * returns:
 *  integer indicating registration success (1) or failure (0).
 */
int vfl_register_factor_type (const factor_type_t *type) {
  /* check the type structure pointer. */
  if (!type)
    return 0;

  /* resize the factor type array. */
  n_factor_types++;
  const unsigned int bytes = n_factor_types * sizeof(factor_type_t*);
  factor_types = (factor_type_t**) realloc(factor_types, bytes);
  if (!factor_types)
    return 0;

  /* store the new type and return success. */
  factor_types[n_factor_types - 1] = (factor_type_t*) type;
  return 1;
}

/* vfl_alloc_model(): allocate a new model by its type name.
 *
 * arguments:
 *  @name: model type name string.
 *
 * returns:
 *  newly allocated and initialized model structure pointer.
 */
model_t *vfl_alloc_model (const char *name) {
  /* search the model type registry. */
  for (unsigned int i = 0; i < n_model_types; i++) {
    /* if a matching name is found, allocate using this type. */
    if (strcmp(model_types[i]->name, name) == 0)
      return model_alloc(model_types[i]);
  }

  /* no match. return null. */
  return NULL;
}

/* vfl_alloc_optim(): allocate a new optimizer by its type name.
 *
 * arguments:
 *  @name: optimizer type name string.
 *
 * returns:
 *  newly allocated and initialized optimizer structure pointer.
 */
optim_t *vfl_alloc_optim (const char *name) {
  /* search the optimizer type registry. */
  for (unsigned int i = 0; i < n_optim_types; i++) {
    /* if a matching name is found, allocate using this type. */
    if (strcmp(optim_types[i]->name, name) == 0)
      return optim_alloc(optim_types[i]);
  }

  /* no match. return null. */
  return NULL;
}

/* vfl_alloc_factor(): allocate a new factor by its type name.
 *
 * arguments:
 *  @name: factor type name string.
 *
 * returns:
 *  newly allocated and initialized factor structure pointer.
 */
factor_t *vfl_alloc_factor (const char *name) {
  /* search the factor type registry. */
  for (unsigned int i = 0; i < n_factor_types; i++) {
    /* if a matching name is found, allocate using this type. */
    if (strcmp(factor_types[i]->name, name) == 0)
      return factor_alloc(factor_types[i]);
  }

  /* no match. return null. */
  return NULL;
}

