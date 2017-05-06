
/* ensure once-only inclusion. */
#ifndef __VFL_MAP_H__
#define __VFL_MAP_H__

/* include c library headers. */
#include <stdlib.h>
#include <string.h>

/* include vfl headers. */
#include <vfl/base/object.h>

/* OBJECT_IS_MAP(): check if an object is a map.
 */
#define OBJECT_IS_MAP(obj) \
  (OBJECT_TYPE(obj) == vfl_object_map)

/* map_pair_t: structure for holding a single key-value pair
 * within a mapping.
 */
typedef struct {
  /* key-value pair:
   *  @key: key string.
   *  @val: object value.
   */
  char *key;
  object_t *val;
}
map_pair_t;

/* map_t: structure for holding an associative array of objects.
 */
typedef struct {
  /* base structure members. */
  OBJECT_BASE;

  /* map contents:
   *  @pairs: array of key-value pairs.
   *  @len: number of key-value pairs.
   */
  map_pair_t *pairs;
  size_t len;
}
map_t;

/* function declarations (base/map.c): */

#define map_alloc() \
  (map_t*) obj_alloc(vfl_object_map)

const char *map_key (const map_t *map, const size_t idx);

object_t *map_val (const map_t *map, const size_t idx);

object_t *map_get (const map_t *map, const char *key);

int map_set (map_t *map, const char *key, object_t *val);

/* available object types: */

extern const object_type_t *vfl_object_map;

#endif /* !__VFL_MAP_H__ */

