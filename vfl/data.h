
/* ensure once-only inclusion. */
#ifndef __VFL_DATA_H__
#define __VFL_DATA_H__

/* include vfl headers. */
#include <vfl/util/matrix.h>
#include <vfl/datum.h>

/* Data_Check(): macro to check if a PyObject is Data.
 */
#define Data_Check(v) (Py_TYPE(v) == &Data_Type)

/* Data: structure for holding observations.
 */
typedef struct {
  /* object base. */
  PyObject_HEAD

  /* dataset size parameters:
   *  @N: number of observations.
   *  @D: number of dimensions.
   */
  size_t N, D;

  /* core dataset array:
   *  @data: array of observations.
   *  @swp: storage for swapping.
   */
  Datum *data;
  Datum swp;
}
Data;

/* function declarations, allocation (data-alloc.c): */

int data_resize (Data *dat, size_t N, size_t D);

/* function declarations (data-entries.c): */

double data_inner (const Data *dat);

Datum *data_get (const Data *dat, size_t i);

int data_set (Data *dat, size_t i, const Datum *d);

size_t data_find (const Data *dat, const Datum *d);

int data_augment (Data *dat, const Datum *d);

int data_augment_from_grid (Data *dat, size_t p, const Matrix *grid);

int data_augment_from_data (Data *dat, const Data *dsrc);

/* function declarations, input/output (data-fileio.c): */

int data_fread (Data *dat, const char *fname);

int data_fwrite (const Data *dat, const char *fname);

/* function declarations, sorting (data-sort.c): */

int data_sort_single (Data *dat, size_t i);

int data_sort (Data *dat);

#endif /* !__VFL_DATA_H__ */

