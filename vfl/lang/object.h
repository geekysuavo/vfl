
/* ensure once-only inclusion. */
#ifndef __VFL_OBJECT_H__
#define __VFL_OBJECT_H__

/* include c library headers. */
#include <stdlib.h>

/* OBJECT_TYPE(): macro function for casting object structure pointers
 * to their associated base type structures.
 */
#define OBJECT_TYPE(s) ((object_type_t*) (s)->type)

/* object_t: defined type for the base object structure. */
typedef struct object object_t;

/* vfl_func(): execute a function from the vfl interpreter.
 *
 * arguments:
 *  @argin: input argument, could be a list.
 *  @argout: pointer to the output argument.
 *
 * returns:
 *  integer indicating function success (1) or failure (0).
 */
typedef int (*vfl_func) (const object_t *argin, object_t **argout);

/* object_init_fn(): initialize the contents of an object.
 *
 * arguments:
 *  @obj: object structure pointer to initialize.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
typedef int (*object_init_fn) (object_t *obj);

/* object_copy_fn(): copy allocated contents between objects.
 *
 * arguments:
 *  @obj: source object structure pointer.
 *  @objdup: destination object structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
typedef int (*object_copy_fn) (const object_t *obj,
                               object_t *objdup);

/* object_free_fn(): free any allocated contents of an object.
 *
 * arguments:
 *  @obj: object structure pointer to free.
 */
typedef void (*object_free_fn) (object_t *obj);

/* object_method_t: structure containing information about
 * a callable method.
 */
typedef struct {
  /* method attributes:
   *  @name: method string name, for lookup.
   *  @fn: method function pointer, for calling.
   */
  const char *name;
  vfl_func fn;
}
object_method_t;

/* object_type_t: structure for holding type-specific information.
 */
typedef struct {
  /* basic object parameters:
   *  @name: string name of the allocated object.
   *  @size: number of bytes allocated to the structure pointer.
   */
  const char *name;
  long size;

  /* core object functions:
   *  @init: initialization hook.
   *  @copy: deep copying hook.
   *  @free: deallocation hook.
   */
  object_init_fn init;
  object_copy_fn copy;
  object_free_fn free;

  /* @methods: object methods table. */
  object_method_t *methods;
}
object_type_t;

/* struct object: structure for holding a vfl object.
 */
struct object {
  /* @type: object type information. */
  object_type_t *type;

  /* object members are placed here. */
};

/* function declarations (lang/object.c): */

object_t *obj_alloc (const object_type_t *type);

object_t *obj_copy (const object_t *obj);

void obj_free (object_t *obj);

#endif /* !__VFL_OBJECT_H__ */

