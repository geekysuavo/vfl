
/* ensure once-only inclusion. */
#ifndef __VFL_OBJECT_H__
#define __VFL_OBJECT_H__

/* include c library headers. */
#include <stdlib.h>

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

  /* @methods: object methods table. */
  object_method_t *methods;
}
object_type_t;

/* struct object: structure for holding a vfl object.
 */
struct object {
  /* @type: object type fields. */
  object_type_t type;

  /* object members are placed here. */
};

/* function declarations (lang/obj.c): */

int vfl_init (void);

int vfl_register_type (const object_type_t *type);

object_t *vfl_alloc (const char *name);

#endif /* !__VFL_OBJECT_H__ */

