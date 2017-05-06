
/* include the map header. */
#include <vfl/base/map.h>

/* map_init(): initialize a mapping.
 *
 * arguments:
 *  @map: mapping structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int map_init (map_t *map) {
  /* initialize the key-value pair array and return success. */
  map->pairs = NULL;
  map->len = 0;
  return 1;
}

/* map_free(): free the contents of a mapping.
 *
 * arguments:
 *  @map: mapping structure pointer to free.
 */
void map_free (map_t *map) {
  /* loop over the key-value pairs. */
  for (size_t i = 0; i < map->len; i++) {
    /* free the key string. */
    free(map->pairs[i].key);

    /* release our reference to the value. */
    obj_release(map->pairs[i].val);
  }

  /* free the key-value pair array. */
  free(map->pairs);
}

/* map_key(): get the key string of an indexed map entry.
 *
 * arguments:
 *  @map: mapping structure pointer.
 *  @idx: key-value pair index.
 *
 * returns:
 *  key string, or null on failure.
 */
const char *map_key (const map_t *map, const size_t idx) {
  /* if possible, return the key string. */
  if (map && idx < map->len)
    return map->pairs[idx].key;

  /* index out of bounds. */
  return NULL;
}

/* map_val(): get the value object of an indexed map entry.
 *
 * arguments:
 *  @map: mapping structure pointer.
 *  @idx: key-value pair index.
 *
 * returns:
 *  value object, or null on failure.
 */
object_t *map_val (const map_t *map, const size_t idx) {
  /* if possible, return the key string. */
  if (map && idx < map->len)
    return map->pairs[idx].val;

  /* index out of bounds. */
  return NULL;
}

/* map_get(): get the object value associated with a key
 * within a mapping structure.
 *
 * arguments:
 *  @map: mapping structure pointer.
 *  @key: key string.
 *
 * returns:
 *  mapped object value associated with the key.
 */
object_t *map_get (const map_t *map, const char *key) {
  /* check the input pointers. */
  if (!map || !key)
    return NULL;

  /* search for the key string. */
  for (size_t i = 0; i < map->len; i++) {
    /* on match, return the mapped value. */
    if (strcmp(map->pairs[i].key, key) == 0)
      return map->pairs[i].val;
  }

  /* no match. */
  return NULL;
}

/* map_set(): set the object value associated with a key
 * within a mapping structure.
 *
 * arguments:
 *  @map: mapping structure pointer.
 *  @key: key string.
 *  @val: value object.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int map_set (map_t *map, const char *key, object_t *val) {
  /* check the input pointers. */
  if (!map || !key)
    return 0;

  /* search for the key string. */
  for (size_t i = 0; i < map->len; i++) {
    /* on match, store the new mapped value. */
    if (strcmp(map->pairs[i].key, key) == 0 &&
        map->pairs[i].val != val) {
      /* release the currently mapped value. */
      obj_release(map->pairs[i].val);

      /* store the new mapped value. */
      map->pairs[i].val = val;
      obj_retain(val);
      return 1;
    }
  }

  /* no match. reallocate the pairs array. */
  const size_t n = map->len;
  const size_t sz = (n + 1) * sizeof(map_pair_t);
  map_pair_t *pairs = realloc(map->pairs, sz);

  /* allocate a key string. */
  char *dupkey = malloc(strlen(key) + 1);

  /* handle allocation failures. */
  if (!pairs || !dupkey)
    return 0;

  /* store the new pair information. */
  strcpy(dupkey, key);
  pairs[n].key = dupkey;
  pairs[n].val = val;
  obj_retain(val);

  /* store the new pairs array. */
  map->len = n + 1;
  map->pairs = pairs;

  /* return success. */
  return 1;
}

/* map_type: mapping type structure.
 */
static object_type_t map_type = {
  "map",                                         /* name      */
  sizeof(map_t),                                 /* size      */

  (object_init_fn) map_init,                     /* init      */
  NULL,                                          /* copy      */
  (object_free_fn) map_free,                     /* free      */

  NULL,                                          /* add       */
  NULL,                                          /* sub       */
  NULL,                                          /* mul       */
  NULL,                                          /* div       */
  NULL,                                          /* pow       */

  NULL,                                          /* get       */
  NULL,                                          /* set       */
  NULL,                                          /* props     */
  NULL                                           /* methods   */
};

/* vfl_object_map: address of the map_type structure. */
const object_type_t *vfl_object_map = &map_type;

