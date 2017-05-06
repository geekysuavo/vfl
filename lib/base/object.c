
/* include the object header. */
#include <vfl/base/object.h>

/* obj_alloc(): allocate a new object by its type structure pointer.
 *
 * arguments:
 *  @type: object type structure pointer.
 *
 * returns:
 *  newly allocated and initialized object structure pointer.
 */
object_t *obj_alloc (const object_type_t *type) {
  /* check the type structure pointer. */
  if (!type)
    return NULL;

  /* allocate a new object. */
  object_t *obj = malloc(type->size);
  if (!obj)
    return NULL;

  /* store the object type. */
  obj->type = (object_type_t*) type;

  /* initialize the reference count to zero. this means that an
   * object must have its reference count incremented in order
   * for it to persist.
   */
  obj->refs = 0;

  /* call the object initialization function. */
  object_init_fn init_fn = type->init;
  if (init_fn && !init_fn(obj)) {
    /* on failure, free and return null. */
    free(obj);
    return NULL;
  }

  /* return the newly allocated and initialized object. */
  return obj;
}

/* obj_copy(): create a copy of an object.
 *
 * arguments:
 *  @obj: object structure pointer to copy.
 *
 * returns:
 *  newly allocated and initialized duplicate of the input object.
 */
object_t *obj_copy (const object_t *obj) {
  /* return null if the input object pointer is null. */
  if (!obj)
    return NULL;

  /* allocate an object of the same type. */
  const object_type_t *type = OBJECT_TYPE(obj);
  object_t *objdup = obj_alloc(type);
  if (!objdup)
    return NULL;

  /* call the object copy function. */
  object_copy_fn copy_fn = type->copy;
  if (copy_fn && !copy_fn(obj, objdup)) {
    /* on failure, free and return null. */
    obj_free(objdup);
    return NULL;
  }

  /* return the duplicate object. */
  return objdup;
}

/* obj_release(): release a reference to an object.
 *
 * arguments:
 *  @obj: pointer to the object structure.
 */
void obj_release (object_t *obj) {
  /* return if the object is null. */
  if (!obj)
    return;

  /* decrement the reference count. */
  if (obj->refs)
    obj->refs--;

  /* check if the reference count is zero. */
  if (obj->refs == 0) {
    /* free the object. */
    obj_free(obj);
    return;
  }
}

/* obj_free(): free an allocated object.
 *
 * arguments:
 *  @obj: pointer to the object structure to free.
 */
void obj_free (object_t *obj) {
  /* return if the object pointer is null. */
  if (!obj || OBJECT_IS_NIL(obj))
    return;

  /* call the object destruction function. */
  object_free_fn free_fn = OBJECT_TYPE(obj)->free;
  if (free_fn)
    free_fn(obj);

  /* free the structure pointer. */
  free(obj);
}

/* obj_add(): perform object addition.
 *  - see object_binary_fn() for details.
 */
object_t *obj_add (const object_t *a, const object_t *b) {
  /* check the input arguments. */
  if (!a || !b)
    return NULL;

  /* get the argument object types. */
  const object_type_t *ta = OBJECT_TYPE(a);
  const object_type_t *tb = OBJECT_TYPE(b);

  /* try the first type function. */
  object_t *c = NULL;
  if (ta->add)
    c = ta->add(a, b);

  /* if unsuccessful, try the second type function. */
  if (!c && tb->add)
    c = tb->add(a, b);

  /* return the result. */
  return c;
}

/* obj_sub(): perform object subtraction.
 *  - see object_binary_fn() for details.
 */
object_t *obj_sub (const object_t *a, const object_t *b) {
  /* check the input arguments. */
  if (!a || !b)
    return NULL;

  /* get the argument object types. */
  const object_type_t *ta = OBJECT_TYPE(a);
  const object_type_t *tb = OBJECT_TYPE(b);

  /* try the first type function. */
  object_t *c = NULL;
  if (ta->sub)
    c = ta->sub(a, b);

  /* if unsuccessful, try the second type function. */
  if (!c && tb->sub)
    c = tb->sub(a, b);

  /* return the result. */
  return c;
}

/* obj_mul(): perform object multiplication.
 *  - see object_binary_fn() for details.
 */
object_t *obj_mul (const object_t *a, const object_t *b) {
  /* check the input arguments. */
  if (!a || !b)
    return NULL;

  /* get the argument object types. */
  const object_type_t *ta = OBJECT_TYPE(a);
  const object_type_t *tb = OBJECT_TYPE(b);

  /* try the first type function. */
  object_t *c = NULL;
  if (ta->mul)
    c = ta->mul(a, b);

  /* if unsuccessful, try the second type function. */
  if (!c && tb->mul)
    c = tb->mul(a, b);

  /* return the result. */
  return c;
}

/* obj_div(): perform object division.
 *  - see object_binary_fn() for details.
 */
object_t *obj_div (const object_t *a, const object_t *b) {
  /* check the input arguments. */
  if (!a || !b)
    return NULL;

  /* get the argument object types. */
  const object_type_t *ta = OBJECT_TYPE(a);
  const object_type_t *tb = OBJECT_TYPE(b);

  /* try the first type function. */
  object_t *c = NULL;
  if (ta->div)
    c = ta->div(a, b);

  /* if unsuccessful, try the second type function. */
  if (!c && tb->div)
    c = tb->div(a, b);

  /* return the result. */
  return c;
}

/* obj_pow(): perform object exponentiation.
 *  - see object_binary_fn() for details.
 */
object_t *obj_pow (const object_t *a, const object_t *b) {
  /* check the input arguments. */
  if (!a || !b)
    return NULL;

  /* get the argument object types. */
  const object_type_t *ta = OBJECT_TYPE(a);
  const object_type_t *tb = OBJECT_TYPE(b);

  /* try the first type function. */
  object_t *c = NULL;
  if (ta->pow)
    c = ta->pow(a, b);

  /* if unsuccessful, try the second type function. */
  if (!c && tb->pow)
    c = tb->pow(a, b);

  /* return the result. */
  return c;
}

/* obj_getprop(): get the value of an object property.
 *
 * arguments:
 *  @obj: object structure pointer.
 *  @name: property name.
 *
 * returns:
 *  requested property value, or null.
 */
object_t *obj_getprop (const object_t *obj, const char *name) {
  /* check the input arguments. */
  if (!obj || !name)
    return NULL;

  /* get the object type and check for a property table. */
  const object_type_t *type = OBJECT_TYPE(obj);
  if (!type->props)
    return NULL;

  /* search for the named property. */
  for (size_t i = 0; type->props[i].name; i++) {
    /* on match, execute the property getter. */
    if (strcmp(type->props[i].name, name) == 0 && type->props[i].get)
      return type->props[i].get(obj);
  }

  /* no getter found. */
  return NULL;
}

/* obj_getelem(): get the value of an object element.
 *
 * arguments:
 *  @obj: object structure pointer.
 *  @idx: element index.
 *
 * returns:
 *  requested element value, or null.
 */
object_t *obj_getelem (const object_t *obj, const object_t *idx) {
  /* check the input arguments. */
  if (!obj || !idx)
    return NULL;

  /* get the object type and check for a getter. */
  const object_type_t *type = OBJECT_TYPE(obj);
  if (type->get)
    return type->get(obj, idx);

  /* no getter available. */
  return NULL;
}

/* obj_setprop(): set the value of an object property.
 *
 * arguments:
 *  @obj: object structure pointer.
 *  @name: property name.
 *  @val: property value.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int obj_setprop (object_t *obj, const char *name, object_t *val) {
  /* check the input arguments. */
  if (!obj || !name || !val)
    return 0;

  /* get the object type and check for a property table. */
  const object_type_t *type = OBJECT_TYPE(obj);
  if (!type->props)
    return 0;

  /* search for the named property. */
  for (size_t i = 0; type->props[i].name; i++) {
    /* on match, execute the property setter. */
    if (strcmp(type->props[i].name, name) == 0 && type->props[i].set)
      return type->props[i].set(obj, val);
  }

  /* no setter found. */
  return 0;
}

/* obj_setelem(): set the value of an object element.
 *
 * arguments:
 *  @obj: object structure pointer.
 *  @idx: element index.
 *  @val: element value.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int obj_setelem (object_t *obj, const object_t *idx,
                 object_t *val) {
  /* check the input arguments. */
  if (!obj || !idx || !val)
    return 0;

  /* get the object type and check for a setter. */
  const object_type_t *type = OBJECT_TYPE(obj);
  if (type->set)
    return type->set(obj, idx, val);

  /* no setter available. */
  return 0;
}

/* obj_method(): call an object method.
 *
 * arguments:
 *  @obj: object structure pointer.
 *  @name: method name.
 *  @args: arguments.
 *
 * returns:
 *  result of the called method, or null.
 */
object_t *obj_method (object_t *obj, const char *name, object_t *args) {
  /* check the input arguments. */
  if (!obj || !name || !args)
    return NULL;

  /* get the object type and check for a method table. */
  const object_type_t *type = OBJECT_TYPE(obj);
  if (!type->methods)
    return NULL;

  /* search for the named method. */
  for (size_t i = 0; type->methods[i].name; i++) {
    /* on match, execute the method. */
    if (strcmp(type->methods[i].name, name) == 0)
      return type->methods[i].fn(obj, args);
  }

  /* no method found. */
  return NULL;
}

