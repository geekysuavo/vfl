
/* ensure once-only inclusion. */
#ifndef __VFL_DATA_H__
#define __VFL_DATA_H__

/* include c headers. */
#include <stdio.h>
#include <stdlib.h>

/* include application headers. */
#include <vfl/util/matrix.h>
#include <vfl/util/vector.h>

/* data_t: structure for holding observations.
 */
typedef struct {
  /* dataset size parameters:
   *  @N: number of observations.
   *  @D: number of dimensions.
   */
  unsigned int N, D;

  /* core dataset arrays:
   *  @X: observation inputs.
   *  @y: observed outputs.
   */
  matrix_t *X;
  vector_t *y;
}
data_t;

/* function declarations (data.c): */

data_t *data_alloc (void);

data_t *data_alloc_from_grid (const matrix_t *grid);

void data_free (data_t *dat);

int data_get (const data_t *dat, const unsigned int i,
              vector_t *x, double *y);

int data_set (data_t *dat, const unsigned int i,
              const vector_t *x, const double y);

int data_augment (data_t *dat, const vector_t *x, const double y);

int data_augment_from_grid (data_t *dat, const matrix_t *grid);

int data_fread (data_t *dat, const char *fname);

int data_fwrite (const data_t *dat, const char *fname);

#endif /* !__VFL_DATA_H__ */

