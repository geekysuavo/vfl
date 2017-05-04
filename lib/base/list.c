
/* include the list and integer headers. */
#include <vfl/base/list.h>
#include <vfl/base/float.h>

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

/* --- */

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

/* list_deepfree(): free the contents of an object list, including
 * the list elements themselves.
 *
 * arguments:
 *  @lst: list structure pointer to free.
 */
void list_deepfree (list_t *lst) {
  /* return if the list is null. */
  if (!lst)
    return;

  /* free the list elements. */
  for (size_t i = 0; i < lst->len; i++)
    obj_free(lst->objs[i]);

  /* free the list object. */
  obj_free((object_t*) lst);
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

/* list_alloc_from_vector(): allocate an object list that holds
 * the contents of a vector.
 *
 * arguments:
 *  @v: vector structure pointer to access.
 *
 * returns:
 *  newly allocated and initialized list, or null on failure.
 */
object_t *list_alloc_from_vector (const vector_t *v) {
  /* check the input pointer. */
  if (!v)
    return NULL;

  /* return nothing if the vector is empty. */
  if (v->len == 0)
    VFL_RETURN_NIL;

  /* allocate a new list. */
  list_t *lst = list_alloc_with_length(v->len);
  if (!lst)
    return NULL;

  /* allocate the list elements. */
  for (size_t i = 0; i < lst->len; i++) {
    /* allocate and store the element. */
    lst->objs[i] = (object_t*) float_alloc_with_value(vector_get(v, i));
    if (!lst->objs[i])
      goto fail;
  }

  /* return the new list. */
  return (object_t*) lst;

fail:
  /* free all allocated objects and return null. */
  list_deepfree(lst);
  return NULL;
}

/* list_alloc_from_matrix(): allocate an object list that holds
 * the contents of a matrix.
 *
 * arguments:
 *  @A: matrix structure pointer to access.
 *
 * returns:
 *  newly allocated and initialized list, or null on failure.
 */
object_t *list_alloc_from_matrix (const matrix_t *A) {
  /* check the input pointer. */
  if (!A)
    return NULL;

  /* return nothing if the matrix is empty. */
  if (A->rows == 0 || A->cols == 0)
    VFL_RETURN_NIL;

  /* allocate the row list. */
  list_t *rows = list_alloc_with_length(A->rows);
  if (!rows)
    return NULL;

  /* allocate each row. */
  for (size_t i = 0; i < rows->len; i++) {
    /* build a list of the current row elements. */
    vector_view_t Arow = matrix_row(A, i);
    rows->objs[i] = list_alloc_from_vector(&Arow);
    if (!rows->objs[i])
      goto fail;
  }

  /* return the new list. */
  return (object_t*) rows;

fail:
  /* free all row lists. */
  for (size_t i = 0; i < rows->len; i++)
    list_deepfree((list_t*) rows->objs[i]);

  /* free all the row and return null. */
  list_deepfree(rows);
  return NULL;
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

/* list_to_vector(): cast a list into a vector, if possible.
 *
 * arguments:
 *  @lst: object list to access.
 *
 * returns:
 *  vector of the list elements, if the list contained only
 *  integers and/or floats.
 */
vector_t *list_to_vector (const list_t *lst) {
  /* check the list structure pointer. */
  if (!lst)
    return NULL;

  /* get the vector length. */
  const size_t len = lst->len;
  if (len == 0)
    return NULL;

  /* verify that all list elements are numbers. */
  for (size_t i = 0; i < len; i++) {
    /* fail on encountering a non-numeric element. */
    if (!OBJECT_IS_NUM(lst->objs[i]))
      return NULL;
  }

  /* allocate a vector to store the list elements. */
  vector_t *v = vector_alloc(len);
  if (!v)
    return NULL;

  /* transfer the list contents to the vector. */
  for (size_t i = 0; i < len; i++)
    vector_set(v, i, num_get(lst->objs[i]));

  /* return the new vector. */
  return v;
}

/* list_to_matrix(): cast a list into a matrix, if possible.
 *
 * arguments:
 *  @lst: object list to access.
 *
 * returns:
 *  matrix of the list elements, if the list contained only
 *  equally-sized lists of integers and/or floats.
 */
matrix_t *list_to_matrix (const list_t *lst) {
  /* check the list structure pointer. */
  if (!lst)
    return NULL;

  /* get the matrix row count. */
  const size_t rows = lst->len;
  if (rows == 0)
    return NULL;

  /* verify that all list elements are lists. */
  for (size_t i = 0; i < rows; i++) {
    /* fail on encountering a non-list element. */
    if (!OBJECT_IS_LIST(lst->objs[i]))
      return NULL;
  }

  /* get the first-row length. */
  list_t *row0 = (list_t*) lst->objs[0];
  const size_t cols = row0->len;
  if (cols == 0)
    return NULL;

  /* verify that all row lists have the same length. */
  for (size_t i = 1; i < rows; i++) {
    /* fail on encountering a different length. */
    list_t *row = (list_t*) lst->objs[i];
    if (row->len != cols)
      return NULL;
  }

  /* verify that all row lists contain only numbers. */
  for (size_t i = 0; i < rows; i++) {
    /* verify the current row elements. */
    list_t *row = (list_t*) lst->objs[i];
    for (size_t j = 0; j < cols; j++) {
      /* fail on encountering a non-numeric row element. */
      object_t *elem = row->objs[j];
      if (!OBJECT_IS_NUM(elem))
        return NULL;
    }
  }

  /* allocate a matrix to store the list elements. */
  matrix_t *A = matrix_alloc(rows, cols);
  if (!A)
    return NULL;

  /* transfer the list contents to the matrix. */
  for (size_t i = 0; i < rows; i++) {
    /* transfer the current row elements. */
    list_t *row = (list_t*) list_get(lst, i);
    for (size_t j = 0; j < cols; j++)
      matrix_set(A, i, j, num_get(row->objs[j]));
  }

  /* return the new matrix. */
  return A;
}

/* --- */

/* list_add(): addition function for lists.
 *  - see object_binary_fn() for details.
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
 *  - see object_binary_fn() for details.
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

/* --- */

/* list_getelem(): element getter for lists.
 *  - see object_getelem_fn() for details.
 */
object_t *list_getelem (const list_t *lst, const list_t *idx) {
  /* only admit single-element indices. */
  if (idx->len != 1)
    return NULL;

  /* only admit integer indices. */
  object_t *idxobj = list_get(idx, 0);
  if (!OBJECT_IS_INT(idxobj))
    return NULL;

  /* perform bounds checking on the index. */
  const long idxval = int_get((int_t*) idxobj);
  if (idxval < 0 || (size_t) idxval >= lst->len)
    return NULL;

  /* return the list element. */
  return list_get(lst, idxval);
}

/* list_setelem(): element setter for lists.
 *  - see object_setelem_fn() for details.
 */
int list_setelem (list_t *lst, list_t *idx, object_t *elem) {
  /* only admit single-element indices. */
  if (idx->len != 1)
    return 0;

  /* only admit integer indices. */
  object_t *idxobj = list_get(idx, 0);
  if (!OBJECT_IS_INT(idxobj))
    return 0;

  /* perform bounds checking on the index. */
  const long ival = int_get((int_t*) idxobj);
  if (ival < 0 || (size_t) ival >= lst->len)
    return 0;

  /* set the list element and return success. */
  list_set(lst, ival, elem);
  return 1;
}

/* --- */

/* list_getprop_len(): length property getter.
 *  - see object_getprop_fn() for details.
 */
static int_t *list_getprop_len (const list_t *lst) {
  /* return the list length as an integer. */
  return int_alloc_with_value(lst->len);
}

/* list_properties: array of accessible object properties.
 */
static object_property_t list_properties[] = {
  { "len", (object_getprop_fn) list_getprop_len, NULL },
  { NULL, NULL, NULL }
};

/* --- */

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

  (object_getelem_fn) list_getelem,              /* get       */
  (object_setelem_fn) list_setelem,              /* set       */
  list_properties,                               /* props     */
  NULL                                           /* methods   */
};

/* vfl_object_list: address of the list_type structure. */
const object_type_t *vfl_object_list = &list_type;

