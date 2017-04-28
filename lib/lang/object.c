
/* include the vfl header. */
#include <vfl/vfl.h>

/* object_types: global registry of all recognized object types.
 */
static object_type_t **object_types;
static unsigned int n_object_types;

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

  /* initialize the type registry. */
  object_types = NULL;
  n_object_types = 0;

  /* register core model types. */
  res &= vfl_register_type(OBJECT_TYPE(vfl_model_vfc));
  res &= vfl_register_type(OBJECT_TYPE(vfl_model_vfr));
  res &= vfl_register_type(OBJECT_TYPE(vfl_model_tauvfr));

  /* register core optimizer types. */
  res &= vfl_register_type(OBJECT_TYPE(vfl_optim_fg));
  res &= vfl_register_type(OBJECT_TYPE(vfl_optim_mf));

  /* register core factor types. */
  res &= vfl_register_type(OBJECT_TYPE(vfl_factor_cosine));
  res &= vfl_register_type(OBJECT_TYPE(vfl_factor_decay));
  res &= vfl_register_type(OBJECT_TYPE(vfl_factor_fixed_impulse));
  res &= vfl_register_type(OBJECT_TYPE(vfl_factor_impulse));
  res &= vfl_register_type(OBJECT_TYPE(vfl_factor_polynomial));
  res &= vfl_register_type(OBJECT_TYPE(vfl_factor_product));

  /* register utility object types. */
  res &= vfl_register_type(vfl_object_rng);
  res &= vfl_register_type(vfl_object_data);
  res &= vfl_register_type(vfl_object_search);

  /* return the result. */
  return res;
}

/* vfl_register_type(): store a new object type in the
 * central type registry.
 *
 * arguments:
 *  @type: object type structure pointer.
 *
 * returns:
 *  integer indicating registration success (1) or failure (0).
 */
int vfl_register_type (const object_type_t *type) {
  /* check the type structure pointer. */
  if (!type)
    return 0;

  /* resize the object type array. */
  n_object_types++;
  const unsigned int bytes = n_object_types * sizeof(object_type_t*);
  object_types = realloc(object_types, bytes);
  if (!object_types)
    return 0;

  /* store the new type and return success. */
  object_types[n_object_types - 1] = (object_type_t*) type;
  return 1;
}

/* vfl_lookup_type(): locate the registered object type structure
 * pointer associated with a given type name, if any.
 *
 * arguments:
 *  @name: object type name string to search for.
 *
 * returns:
 *  object type pointer, or null.
 */
object_type_t *vfl_lookup_type (const char *name) {
  /* check the input string. */
  if (!name)
    return NULL;

  /* loop over the object type registry. */
  for (unsigned int i = 0; i < n_object_types; i++) {
    /* on match, return the type structure pointer. */
    if (strcmp(object_types[i]->name, name) == 0)
      return object_types[i];
  }

  /* no match, return null. */
  return NULL;
}

/* vfl_alloc(): allocate a new object by its type structure pointer.
 *
 * arguments:
 *  @type: object type structure pointer.
 *
 * returns:
 *  newly allocated and initialized object structure pointer.
 */
object_t *vfl_alloc (const object_type_t *type) {
  /* check the type structure pointer. */
  if (!type)
    return NULL;

  /* allocate a new object. */
  object_t *obj = malloc(type->size);
  if (!obj)
    return NULL;

  /* FIXME: call the object initialization function. */

  /* return the newly allocated and initialized object. */
  return obj;
}

