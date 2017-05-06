
/* ensure once-only inclusion. */
#ifndef __VFL_DATA_H__
#define __VFL_DATA_H__

/* include c headers. */
#include <stdio.h>
#include <stdlib.h>

/* include vfl headers. */
#include <vfl/base/object.h>
#include <vfl/util/matrix.h>
#include <vfl/util/vector.h>
#include <vfl/datum.h>

/* OBJECT_IS_DATA(): check if an object is a dataset.
 */
#define OBJECT_IS_DATA(obj) \
  (OBJECT_TYPE(obj) == vfl_object_data)

/* data_t: structure for holding observations.
 */
typedef struct {
  /* base structure members. */
  OBJECT_BASE;

  /* dataset size parameters:
   *  @N: number of observations.
   *  @D: number of dimensions.
   */
  unsigned int N, D;

  /* core dataset array:
   *  @data: array of observations.
   *  @swp: storage for swapping.
   */
  datum_t *data;
  datum_t swp;
}
data_t;

/* function declarations, allocation (data-alloc.c): */

#define data_alloc() \
  (data_t*) obj_alloc(vfl_object_data);

data_t *data_alloc_from_file (const char *fname);

data_t *data_alloc_from_grid (const unsigned int P,
                              const matrix_t *grid);

int data_resize (data_t *dat, const unsigned int N, const unsigned int D);

/* function declarations (data-entries.c): */

double data_inner (const data_t *dat);

datum_t *data_get (const data_t *dat, const unsigned int i);

int data_set (data_t *dat, const unsigned int i, const datum_t *d);

unsigned int data_find (const data_t *dat, const datum_t *d);

int data_augment (data_t *dat, const datum_t *d);

int data_augment_from_grid (data_t *dat, const unsigned int p,
                            const matrix_t *grid);

int data_augment_from_data (data_t *dat, const data_t *dsrc);

/* function declarations, input/output (data-fileio.c): */

int data_fread (data_t *dat, const char *fname);

int data_fwrite (const data_t *dat, const char *fname);

/* function declarations, sorting (data-sort.c): */

int data_sort_single (data_t *dat, const unsigned int i);

int data_sort (data_t *dat);

/* available object types: */

extern const object_type_t *vfl_object_data;

#endif /* !__VFL_DATA_H__ */

