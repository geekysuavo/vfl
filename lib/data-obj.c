
/* include the dataset header. */
#include <vfl/data.h>

/* include the required object headers. */
#include <vfl/base/map.h>
#include <vfl/base/list.h>
#include <vfl/base/float.h>
#include <vfl/base/string.h>

/* data_init(): initialize the contents of a dataset.
 *
 * arguments:
 *  @dat: dataset structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int data_init (data_t *dat) {
  /* initialize the size of the dataset. */
  dat->N = 0;
  dat->D = 0;

  /* initialize the data array. */
  dat->data = NULL;
  dat->swp.x = NULL;

  /* return success. */
  return 1;
}

/* data_free(): free the contents of a dataset.
 *
 * arguments:
 *  @dat: dataset structure pointer to free.
 */
void data_free (data_t *dat) {
  /* free the swap vector. */
  vector_free(dat->swp.x);

  /* free the array of observations. */
  free(dat->data);
}

/* --- */

/* data_getelem(): method for getting dataset entries.
 *  - see object_getelem_fn() for details.
 */
static object_t *data_getelem (const data_t *dat, const list_t *idx) {
  /* only admit single-element indices. */
  if (idx->len != 1)
    return NULL;

  /* only admit integer indices. */
  object_t *idxobj = list_get(idx, 0);
  if (!OBJECT_IS_INT(idxobj))
    return NULL;

  /* perform bounds checking on the index. */
  const long idxval = int_get((int_t*) idxobj);
  if (idxval < 0 || (unsigned int) idxval >= dat->N)
    return NULL;

  /* FIXME: implement data_getelem() */ return NULL;
}

/* data_setelem(): method for setting/modifying dataset entries.
 *  - see object_setelem_fn() for details.
 */
static int data_setelem (data_t *dat, const list_t *idx, object_t *val) {
  /* only admit single-element indices. */
  if (idx->len != 1)
    return 0;

  /* only admit integer indices. */
  object_t *idxobj = list_get(idx, 0);
  if (!OBJECT_IS_INT(idxobj))
    return 0;

  /* perform bounds checking on the index. */
  const long idxval = int_get((int_t*) idxobj);
  if (idxval < 0 || (unsigned int) idxval >= dat->N)
    return 0;

  /* FIXME: implement data_setelem() */ return 0;
}

/* --- */

/* data_getprop_len(): get the observation count of a dataset.
 *  - see object_getprop_fn() for details.
 */
static int_t *data_getprop_len (const data_t *dat) {
  /* return the observation count as an integer. */
  return int_alloc_with_value(dat->N);
}

/* data_getprop_dims(): get the dimension count of a dataset.
 *  - see object_getprop_fn() for details.
 */
static int_t *data_getprop_dims (const data_t *dat) {
  /* return the dimension count as an integer. */
  return int_alloc_with_value(dat->D);
}

/* data_setprop_file(): augment the dataset from a file.
 *  - see object_setprop_fn() for details.
 */
static int data_setprop_file (data_t *dat, object_t *val) {
  /* admit only string filenames. */
  if (!OBJECT_IS_STRING(val))
    return 0;

  /* read the file and return the result. */
  const char *fname = string_get((string_t*) val);
  return data_fread(dat, fname);
}

/* data_setprop_grid(): augment the dataset from gridding information.
 *  - see object_setprop_fn() for details.
 */
static int data_setprop_grid (data_t *dat, object_t *val) {
  /* admit only list grid. */
  if (!OBJECT_IS_LIST(val))
    return 0;

  /* cast the list to a matrix. */
  matrix_t *grid = list_to_matrix((list_t*) val);
  if (!grid)
    return 0;

  /* augment the dataset with the grid. */
  const int ret = data_augment_from_grid(dat, 0, grid);

  /* free the allocated matrix and return. */
  matrix_free(grid);
  return ret;
}

/* data_properties: array of accessible object properties.
 */
static object_property_t data_properties[] = {
  { "N", (object_getprop_fn) data_getprop_len, NULL },
  { "D", (object_getprop_fn) data_getprop_dims, NULL },
  { "file", NULL, (object_setprop_fn) data_setprop_file },
  { "grid", NULL, (object_setprop_fn) data_setprop_grid },
  { NULL, NULL, NULL }
};

/* --- */

/* data_method_augment(): augment a dataset with new points.
 *  - see object_method_fn() for details.
 */
static object_t *data_method_augment (data_t *dat, map_t *args) {
  /* FIXME: implement data_method_augment() */

  /* return nothing. */
  VFL_RETURN_NIL;
}

/* data_method_write(): write a dataset object to a file.
 *  - see object_method_fn() for details.
 */
static object_t *data_method_write (data_t *dat, map_t *args) {
  /* get the filename argument. */
  object_t *arg = map_get(args, "file");
  if (!arg)
    return NULL;

  /* admit only string filenames. */
  if (!OBJECT_IS_STRING(arg))
    return NULL;

  /* write the results to the file. */
  const char *fname = string_get((string_t*) arg);
  data_fwrite(dat, fname);

  /* return nothing. */
  VFL_RETURN_NIL;
}

/* data_methods: array of callable object methods.
 */
static object_method_t data_methods[] = {
  { "augment", (object_method_fn) data_method_augment },
  { "write", (object_method_fn) data_method_write },
  { NULL, NULL }
};

/* --- */

/* data_type: dataset type structure.
 */
static object_type_t data_type = {
  "data",                                        /* name      */
  sizeof(data_t),                                /* size      */

  (object_init_fn) data_init,                    /* init      */
  NULL,                                          /* copy      */
  (object_free_fn) data_free,                    /* free      */

  NULL,                                          /* add       */
  NULL,                                          /* sub       */
  NULL,                                          /* mul       */
  NULL,                                          /* div       */

  (object_getelem_fn) data_getelem,              /* get       */
  (object_setelem_fn) data_setelem,              /* set       */
  data_properties,                               /* props     */
  data_methods                                   /* methods   */
};

/* vfl_object_data: address of the data_type structure. */
const object_type_t *vfl_object_data = &data_type;

