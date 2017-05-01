
/* ensure once-only inclusion. */
#ifndef __VFL_LIST_H__
#define __VFL_LIST_H__

/* include c library headers. */
#include <stdlib.h>

/* include vfl headers. */
#include <vfl/lang/object.h>

/* OBJECT_IS_LIST(): check if an object is a list.
 */
#define OBJECT_IS_LIST(obj) \
  (OBJECT_TYPE(obj) == vfl_object_list)

/* list_t: structure for holding a list of objects.
 */
typedef struct {
  /* @type: basic object type information. */
  object_type_t *type;

  /* list contents:
   *  @objs: array of objects in the list.
   *  @len: number of objects in the list.
   */
  object_t **objs;
  size_t len;
}
list_t;

/* function declarations (util/list.c): */

#define list_alloc() \
  (list_t*) obj_alloc(vfl_object_list)

list_t *list_alloc_with_length (const size_t len);

object_t *list_get (const list_t *lst, const size_t idx);

void list_set (list_t *lst, const size_t idx, const object_t *obj);

/* available object types: */

extern const object_type_t *vfl_object_list;

#endif /* !__VFL_LIST_H__ */

