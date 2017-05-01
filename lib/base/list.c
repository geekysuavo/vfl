
/* include the list and integer headers. */
#include <vfl/base/list.h>
#include <vfl/base/int.h>

/* list_set_length(): set the number of available elements
 * of an object list. this function does not manage the
 * values of the object array; it merely resizes it.
 *
 * arguments:
 *  @lst: list object structure pointer.
 *  @len: new length value to set.
 *
 * returns:
 *  integer indicating resize success (1) or failure (0).
 */
static int list_set_length (list_t *lst, const size_t len) {
  /* check the input pointer. */
  if (!lst)
    return 0;

  /* act based on the new size. */
  if (len > 0) {
    /* reallocate the object array. */
    const size_t sz = len * sizeof(object_t*);
    object_t **objs = realloc(lst->objs, sz);
    if (!objs)
      return 0;

    /* store the new object array. */
    lst->objs = objs;
  }
  else {
    /* free the current list. */
    free(lst->objs);
    lst->objs = NULL;
  }

  /* store the new length and return success. */
  lst->len = len;
  return 1;
}

/* list_init(): initialize an object list.
 *
 * arguments:
 *  @lst: list object structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int list_init (list_t *lst) {
  /* initialize the object array and return success. */
  lst->objs = NULL;
  lst->len = 0;
  return 1;
}

/* list_copy(): copy the contents of one list to another.
 *
 * arguments:
 *  @l: source list structure pointer.
 *  @ldup: destination list structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int list_copy (const list_t *l, list_t *ldup) {
  /* resize the duplicate list. */
  if (!list_set_length(ldup, l->len))
    return 0;

  /* perform a shallow copy. */
  for (size_t i = 0; i < l->len; i++)
    ldup->objs[i] = l->objs[i];

  /* return success. */
  return 1;
}

/* list_free(): free the contents of an object list.
 *
 * arguments:
 *  @lst: list structure pointer to free.
 */
void list_free (list_t *lst) {
  /* free the object array. */
  list_set_length(lst, 0);
}

/* list_alloc_with_length(): allocate an object list
 * with a specified number of array elements.
 *
 * arguments:
 *  @len: number of array elements to allocate.
 *
 * returns:
 *  newly allocated and resized list, or null on failure.
 */
list_t *list_alloc_with_length (const size_t len) {
  /* allocate a new list. */
  list_t *lst = list_alloc();
  if (!lst)
    return NULL;

  /* resize the list. */
  if (!list_set_length(lst, len)) {
    obj_free((object_t*) lst);
    return NULL;
  }

  /* initialize the list elements. */
  for (size_t i = 0; i < lst->len; i++)
    lst->objs[i] = NULL;

  /* return the new list. */
  return lst;
}

/* list_get(): get an element from an object list.
 *
 * arguments:
 *  @lst: object list to access.
 *  @idx: list index to get.
 *
 * returns:
 *  value of the list element.
 */
object_t *list_get (const list_t *lst, const size_t idx) {
  /* return the list element. */
  return (lst && idx < lst->len ? lst->objs[idx] : NULL);
}

/* list_set(): set an element of an object list.
 *
 * arguments:
 *  @lst: object list to modify.
 *  @idx: list index to set.
 *  @obj: new element value.
 */
void list_set (list_t *lst, const size_t idx, const object_t *obj) {
  /* if possible, set the list element. */
  if (lst && idx < lst->len)
    lst->objs[idx] = (object_t*) obj;
}

/* list_append(): add an object to the end of a list.
 *
 * arguments:
 *  @lst: object list to modify.
 *  @obj: object to append.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int list_append (list_t *lst, object_t *obj) {
  /* resize the list. */
  if (!lst || !list_set_length(lst, lst->len + 1))
    return 0;

  /* store the new element and return success. */
  lst->objs[lst->len - 1] = obj;
  return 1;
}

/* list_prepend(): add an object to the beginning of a list.
 *
 * arguments:
 *  @lst: object list to modify.
 *  @obj: object to prepend.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int list_prepend (list_t *lst, object_t *obj) {
  /* resize the list. */
  if (!lst || !list_set_length(lst, lst->len + 1))
    return 0;

  /* shift all elements down by one. */
  for (size_t i = lst->len - 1; i > 0; i--)
    lst->objs[i] = lst->objs[i - 1];

  /* store the new element and return success. */
  lst->objs[0] = obj;
  return 1;
}

/* list_add(): addition function for lists.
 */
list_t *list_add (const object_t *a, const object_t *b) {
  /* check the argument types. */
  if (OBJECT_IS_LIST(a) && OBJECT_IS_LIST(b)) {
    /* cast the input arguments to lists. */
    const list_t *la = (list_t*) a;
    const list_t *lb = (list_t*) b;

    /* allocate a new list to store the concatenated lists. */
    list_t *lst = list_alloc_with_length(la->len + lb->len);
    if (!lst)
      return NULL;

    /* shallow copy the first list elements. */
    size_t idx = 0;
    for (size_t i = 0; i < la->len; i++, idx++)
      lst->objs[idx] = la->objs[i];

    /* shallow copy the second list elements. */
    for (size_t i = 0; i < lb->len; i++, idx++)
      lst->objs[idx] = lb->objs[i];

    /* return the new list. */
    return lst;
  }
  else if (OBJECT_IS_LIST(a)) {
    /* append the object to the list. */
    list_t *lst = (list_t*) obj_copy(a);
    if (lst && list_append(lst, (object_t*) b))
      return lst;
  }
  else if (OBJECT_IS_LIST(b)) {
    /* prepend the object to the list. */
    list_t *lst = (list_t*) obj_copy(b);
    if (lst && list_prepend(lst, (object_t*) a))
      return lst;
  }

  /* return no result. */
  return NULL;
}

/* list_mul(): multiplication function for lists.
 */
list_t *list_mul (const object_t *a, const object_t *b) {
  /* declare required variables:
   *  @lobj: list object entering the product.
   *  @iobj: integer object multiplier.
   */
  list_t *lobj = NULL;
  int_t *iobj = NULL;

  /* check the argument types. */
  if (OBJECT_IS_LIST(a) && OBJECT_IS_INT(b)) {
    lobj = (list_t*) a;
    iobj = (int_t*) b;
  }
  else if (OBJECT_IS_INT(a) && OBJECT_IS_LIST(b)) {
    lobj = (list_t*) b;
    iobj = (int_t*) a;
  }
  else
    return NULL;

  /* get the integer value. */
  const long ival = int_get(iobj);
  if (ival <= 0)
    return NULL;

  /* determine the new lengths. */
  const size_t len = lobj->len;
  const size_t newlen = len * ival;

  /* allocate a new list. */
  list_t *lst = list_alloc_with_length(newlen);
  if (!lst)
    return NULL;

  /* create copies of the list elements in the proper pattern. */
  for (size_t i = 0, idx = 0; i < (size_t) ival; i++)
    for (size_t j = 0; j < len; j++, idx++)
      lst->objs[idx] = obj_copy(lobj->objs[j]);

  /* return the new list. */
  return lst;
}

/* list_type: list type structure.
 */
static object_type_t list_type = {
  "list",                                        /* name      */
  sizeof(list_t),                                /* size      */

  (object_init_fn) list_init,                    /* init      */
  (object_copy_fn) list_copy,                    /* copy      */
  (object_free_fn) list_free,                    /* free      */

  (object_binary_fn) list_add,                   /* add       */
  NULL,                                          /* sub       */
  (object_binary_fn) list_mul,                   /* mul       */
  NULL,                                          /* div       */

  NULL                                           /* methods   */
};

/* vfl_object_list: address of the list_type structure. */
const object_type_t *vfl_object_list = &list_type;

