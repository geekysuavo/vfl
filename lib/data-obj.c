
/* include the dataset header. */
#include <vfl/data.h>
#include <vfl/base/map.h>
#include <vfl/base/int.h>
#include <vfl/base/list.h>
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

/* dataobj_get_len(): get the observation count of a dataset.
 *  - see object_getprop_fn() for details.
 */
static int_t *dataobj_get_len (const data_t *dat) {
  /* return the observation count as an integer. */
  return int_alloc_with_value(dat->N);
}

/* dataobj_get_dims(): get the dimension count of a dataset.
 *  - see object_getprop_fn() for details.
 */
static int_t *dataobj_get_dims (const data_t *dat) {
  /* return the dimension count as an integer. */
  return int_alloc_with_value(dat->D);
}

/* dataobj_set_file(): augment the dataset from a file.
 *  - see object_setprop_fn() for details.
 */
static int dataobj_set_file (data_t *dat, object_t *val) {
  /* admit only string filenames. */
  if (!OBJECT_IS_STRING(val))
    return 0;

  /* read the file and return the result. */
  const char *fname = string_get((string_t*) val);
  return data_fread(dat, fname);
}

/* dataobj_set_grid(): augment the dataset from gridding information.
 *  - see object_setprop_fn() for details.
 */
static int dataobj_set_grid (data_t *dat, object_t *val) {
  /* admit only list grid. */
  if (!OBJECT_IS_LIST(val))
    return 0;

  /* FIXME: implement dataobj_set_grid() */
  return 0;

  /* return success. */
  return 1;
}

/* data_properties: array of accessible object properties.
 */
static object_property_t data_properties[] = {
  { "N", (object_getprop_fn) dataobj_get_len, NULL },
  { "D", (object_getprop_fn) dataobj_get_dims, NULL },
  { "file", NULL, (object_setprop_fn) dataobj_set_file },
  { "grid", NULL, (object_setprop_fn) dataobj_set_grid },
  { NULL, NULL, NULL }
};

/* --- */

/* dataobj_write(): write a dataset object to a file.
 *  - see object_method_fn() for details.
 */
static object_t *dataobj_write (data_t *dat, map_t *args) {
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
  { "write", (object_method_fn) dataobj_write },
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

  NULL,                                          /* get       */
  NULL,                                          /* set       */
  data_properties,                               /* props     */
  data_methods                                   /* methods   */
};

/* vfl_object_data: address of the data_type structure. */
const object_type_t *vfl_object_data = &data_type;

