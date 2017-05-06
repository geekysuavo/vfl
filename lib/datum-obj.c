
/* include the datum header. */
#include <vfl/datum.h>

/* include the required object headers. */
#include <vfl/base/map.h>
#include <vfl/base/list.h>
#include <vfl/base/float.h>
#include <vfl/base/string.h>

/* datum_init(): initialize the contents of a datum.
 *
 * arguments:
 *  @d: datum structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int datum_init (datum_t *d) {
  /* initialize the output index and value. */
  d->y = 0.0;
  d->p = 0;

  /* initialize the location vector. */
  d->x = NULL;

  /* return success. */
  return 1;
}

/* datum_free(): free the contents of a datum.
 *
 * arguments:
 *  @d: datum structure pointer to free.
 */
void datum_free (datum_t *d) {
  /* free the location vector. */
  vector_free(d->x);
}

/* --- */

/* datum_getelem(): method for getting datum location entries.
 *  - see object_getelem_fn() for details.
 */
static object_t *datum_getelem (const datum_t *d, const list_t *idx) {
  /* fail if the datum location is null. */
  if (!d->x)
    return NULL;

  /* only admit single-element indices. */
  if (idx->len != 1)
    return NULL;

  /* only admit integer indices. */
  object_t *idxobj = list_get(idx, 0);
  if (!OBJECT_IS_INT(idxobj))
    return NULL;

  /* perform bounds checking on the index. */
  const long idxval = int_get((int_t*) idxobj);
  if (idxval < 0 || (unsigned int) idxval >= d->x->len)
    return NULL;

  /* return the location vector element as a float. */
  return (object_t*) float_alloc_with_value(vector_get(d->x, idxval));
}

/* datum_setelem(): method for setting datum location entries.
 *  - see object_setelem_fn() for details.
 */
static int datum_setelem (datum_t *d, const list_t *idx, object_t *val) {
  /* fail if the datum location is null. */
  if (!d->x)
    return 0;

  /* only admit single-element indices. */
  if (idx->len != 1)
    return 0;

  /* only admit integer indices. */
  object_t *idxobj = list_get(idx, 0);
  if (!OBJECT_IS_INT(idxobj))
    return 0;

  /* perform bounds checking on the index. */
  const long idxval = int_get((int_t*) idxobj);
  if (idxval < 0 || (unsigned int) idxval >= d->x->len)
    return 0;

  /* only admit numeric values. */
  if (!OBJECT_IS_NUM(val))
    return 0;

  /* store the location vector element and return success. */
  vector_set(d->x, idxval, num_get(val));
  return 1;
}

/* --- */

/* datum_getprop_dims(): get the dimension count of a datum.
 *  - see object_getprop_fn() for details.
 */
static int_t *datum_getprop_dims (const datum_t *d) {
  /* return the dimension count as an integer. */
  return int_alloc_with_value(d->x ? d->x->len : 0);
}

/* datum_getprop_output(): get the output index of a datum.
 *  - see object_getprop_fn() for details.
 */
static int_t *datum_getprop_output (const datum_t *d) {
  /* return the output index as an integer. */
  return int_alloc_with_value(d->p);
}

/* datum_getprop_input(): get the input location of a datum.
 *  - see object_getprop_fn() for details.
 */
static object_t *datum_getprop_input (const datum_t *d) {
  /* return the location vector as a list. */
  return list_alloc_from_vector(d->x);
}

/* datum_getprop_value(): get the observed value of a datum.
 *  - see object_getprop_fn() for details.
 */
static flt_t *datum_getprop_value (const datum_t *d) {
  /* return the observed value as a float. */
  return float_alloc_with_value(d->y);
}

/* datum_setprop_output(): set the output index of a datum.
 *  - see object_setprop_fn() for details.
 */
static int datum_setprop_output (datum_t *d, object_t *val) {
  /* only admit integer values. */
  if (!OBJECT_IS_INT(val))
    return 0;

  /* bounds check the output index. */
  const long pval = int_get((int_t*) val);
  if (pval < 0)
    return 0;

  /* set the output index and return success. */
  d->p = pval;
  return 1;
}

/* datum_setprop_input(): set the input location of a datum.
 *  - see object_setprop_fn() for details.
 */
static int datum_setprop_input (datum_t *d, object_t *val) {
  /* only admit list values. */
  if (!OBJECT_IS_LIST(val))
    return 0;

  /* cast the list to a vector. */
  vector_t *xval = list_to_vector((list_t*) val);
  if (!xval)
    return 0;

  /* check how to handle the new vector. */
  if (d->x) {
    /* check that the lengths match. */
    if (xval->len != d->x->len) {
      vector_free(xval);
      return 0;
    }

    /* copy the vector contents. */
    vector_copy(d->x, xval);
  }
  else {
    /* set the value and return success. */
    d->x = xval;
    return 1;
  }

  /* free the vector and return success. */
  vector_free(xval);
  return 1;
}

/* datum_setprop_value(): set the observed value of a datum.
 *  - see object_setprop_fn() for details.
 */
static int datum_setprop_value (datum_t *d, object_t *val) {
  /* only admit numeric values. */
  if (!OBJECT_IS_NUM(val))
    return 0;

  /* set the observed value and return success. */
  d->y = num_get(val);
  return 1;
}

/* datum_properties: array of accessible object properties.
 */
static object_property_t datum_properties[] = {
  { "D", (object_getprop_fn) datum_getprop_dims, NULL },
  { "p",
    (object_getprop_fn) datum_getprop_output,
    (object_setprop_fn) datum_setprop_output },
  { "output",
    (object_getprop_fn) datum_getprop_output,
    (object_setprop_fn) datum_setprop_output },
  { "x",
    (object_getprop_fn) datum_getprop_input,
    (object_setprop_fn) datum_setprop_input },
  { "input",
    (object_getprop_fn) datum_getprop_input,
    (object_setprop_fn) datum_setprop_input },
  { "y",
    (object_getprop_fn) datum_getprop_value,
    (object_setprop_fn) datum_setprop_value },
  { "value",
    (object_getprop_fn) datum_getprop_value,
    (object_setprop_fn) datum_setprop_value },
  { NULL, NULL, NULL }
};

/* --- */

/* datum_type: datum type structure.
 */
static object_type_t datum_type = {
  "datum",                                       /* name      */
  sizeof(datum_t),                               /* size      */

  (object_init_fn) datum_init,                   /* init      */
  NULL,                                          /* copy      */
  (object_free_fn) datum_free,                   /* free      */

  NULL,                                          /* add       */
  NULL,                                          /* sub       */
  NULL,                                          /* mul       */
  NULL,                                          /* div       */
  NULL,                                          /* pow       */

  (object_getelem_fn) datum_getelem,             /* get       */
  (object_setelem_fn) datum_setelem,             /* set       */
  datum_properties,                              /* props     */
  NULL,                                          /* methods   */
};

/* vfl_object_datum: address of the datum_type structure. */
const object_type_t *vfl_object_datum = &datum_type;

