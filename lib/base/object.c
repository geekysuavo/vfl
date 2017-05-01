
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

/* obj_free(): free an allocated object.
 *
 * arguments:
 *  @obj: pointer to the object structure to free.
 */
void obj_free (object_t *obj) {
  /* return if the object pointer is null. */
  if (!obj)
    return;

  /* call the object destruction function. */
  object_free_fn free_fn = OBJECT_TYPE(obj)->free;
  if (free_fn)
    free_fn(obj);

  /* free the structure pointer. */
  free(obj);
}

/* obj_add(): perform object addition.
 *  - see object_binary_fn() for more information.
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
 *  - see object_binary_fn() for more information.
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
 *  - see object_binary_fn() for more information.
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
 *  - see object_binary_fn() for more information.
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

