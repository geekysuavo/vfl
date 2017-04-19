
/* ensure once-only inclusion. */
#ifndef __VFL_DATA_H__
#define __VFL_DATA_H__

/* include c headers. */
#include <stdio.h>
#include <stdlib.h>

/* include application headers. */
#include <vfl/util/matrix.h>
#include <vfl/util/vector.h>

/* datum_t: structure for holding a single observation.
 */
typedef struct {
  /* properties of each observation:
   *  @p: observation output index.
   *  @x: observation location.
   *  @y: observed value.
   */
  unsigned int p;
  vector_t *x;
  double y;
}
datum_t;

/* data_t: structure for holding observations.
 */
typedef struct {
  /* dataset size parameters:
   *  @N: number of observations.
   *  @D: number of dimensions.
   */
  unsigned int N, D;

  /* core dataset array:
   *  @data: array of observations.
   */
  datum_t *data;
}
data_t;

/* function declarations, allocation (data-alloc.c): */

data_t *data_alloc (void);

data_t *data_alloc_from_file (const char *fname);

data_t *data_alloc_from_grid (const unsigned int P,
                              const matrix_t *grid);

void data_free (data_t *dat);

int data_resize (data_t *dat, const unsigned int N, const unsigned int D);

/* function declarations (data-entries.c): */

datum_t *data_get (const data_t *dat, const unsigned int i);

int data_set (data_t *dat, const unsigned int i, const datum_t *d);

int data_augment (data_t *dat, const datum_t *d);

int data_augment_from_grid (data_t *dat, const unsigned int p,
                            const matrix_t *grid);

unsigned long data_count_grid (const matrix_t *grid, unsigned int *sz);

/* function declarations, input/output (data-fileio.c): */

int data_fread (data_t *dat, const char *fname);

int data_fwrite (const data_t *dat, const char *fname);

#endif /* !__VFL_DATA_H__ */

