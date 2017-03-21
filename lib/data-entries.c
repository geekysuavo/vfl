
/* include the dataset header. */
#include <vfl/data.h>

/* data_get(): extract an observation from a dataset.
 *
 * arguments:
 *  @dat: dataset structure pointer to access.
 *  @i: observation index to extract.
 *
 * returns:
 *  pointer to the requested observation.
 */
datum_t *data_get (const data_t *dat, const unsigned int i) {
  /* check the input arguments. */
  if (!dat || i >= dat->N)
    return 0;

  /* return the observation. */
  return dat->data + i;
}

/* data_set(): store an observation into a dataset.
 *
 * arguments:
 *  @dat: dataset structure pointer to access.
 *  @i: observation index to extract.
 *  @d: pointer to the observation.
 *
 * returns:
 *  integer indicating whether (1) or not (0) the assignment succeeded.
 */
int data_set (data_t *dat, const unsigned int i, const datum_t *d) {
  /* check the input pointers. */
  if (!dat || !d)
    return 0;

  /* check the index and dimensions. */
  if (i >= dat->N || d->x->len != dat->D)
    return 0;

  /* store the observation. */
  vector_copy(dat->data[i].x, d->x);
  dat->data[i].y = d->y;
  dat->data[i].p = d->p;

  /* return success. */
  return 1;
}

/* data_augment(): add a new observation into a dataset.
 *
 * arguments:
 *  @dat: dataset structure pointer to access.
 *  @d: pointer to the augmenting observation.
 *
 * returns:
 *  integer indicating whether (1) or not (0) the augmentation succeeded.
 */
int data_augment (data_t *dat, const datum_t *d) {
  /* check the input pointers. */
  if (!dat || !d)
    return 0;

  /* check the observation dimensionality. */
  if (dat->D && dat->N && d->x->len != dat->D)
    return 0;

  /* resize the dataset. */
  if (!data_resize(dat, dat->N + 1, d->x->len))
    return 0;

  /* store the augmenting observation. */
  vector_copy(dat->data[dat->N - 1].x, d->x);
  dat->data[dat->N - 1].y = d->y;
  dat->data[dat->N - 1].p = d->p;

  /* return success. */
  return 1;
}

/* data_augment_from_grid(): add a regular grid of zero-valued
 * observations into a dataset.
 *
 * arguments:
 *  @dat: dataset structure pointer to access.
 *  @p: output index of the augmenting grid.
 *  @grid: matrix of gridding information.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int data_augment_from_grid (data_t *dat, const unsigned int p,
                            const matrix_t *grid) {
  /* declare required variables:
   *  @N, @D: new dataset sizes.
   *  @idx: current grid index.
   *  @sz: maximum grid indices.
   *  @x: vector of grid locations.
   */
  unsigned int status, N, D, *idx, *sz;
  vector_t *x;

  /* initialize the return status. */
  status = 0;

  /* initialize the pointers. */
  idx = NULL;
  sz = NULL;
  x = NULL;

  /* check the input pointers. */
  if (!dat || !grid)
    return 0;

  /* check the gridding matrix. */
  if (grid->cols != 3)
    return 0;

  /* initialize the new sizes. */
  D = grid->rows;
  N = 1;

  /* allocate indices for grid traversal. */
  idx = (unsigned int*) malloc(D * sizeof(unsigned int));
  sz = (unsigned int*) malloc(D * sizeof(unsigned int));
  if (!idx || !sz)
    goto fail;

  /* compute the size of the augmenting grid. */
  for (unsigned int d = 0; d < D; d++) {
    /* get the start, end, and step along the current dimension. */
    const double x1 = matrix_get(grid, d, 0);
    const double x2 = matrix_get(grid, d, 2);
    const double dx = matrix_get(grid, d, 1);

    /* compute the grid size along the current dimension. */
    sz[d] = (unsigned int) floor((x2 - x1) / dx) + 1;
    idx[d] = 0;

    /* include the current dimension contribution to the size. */
    N *= sz[d];
  }

  /* attempt to resize the dataset. */
  if (!data_resize(dat, dat->N + N, D))
    goto fail;

  /* allocate a vector for holding observations. */
  x = vector_alloc(D);
  if (!x)
    goto fail;

  /* initialize the observation vector. */
  matrix_copy_col(x, grid, 0);

  /* loop over every grid point. */
  for (unsigned int i = 0; i < N; i++) {
    /* store the current grid point. */
    vector_copy(dat->data[i].x, x);
    dat->data[i].y = 0.0;
    dat->data[i].p = p;

    /* move to the next grid point. */
    for (unsigned int d = 0; d < D; d++) {
      /* get the start and step along the current dimension. */
      const double x1 = matrix_get(grid, d, 0);
      const double dx = matrix_get(grid, d, 1);

      /* increment the current dimension. */
      idx[d]++;

      /* update the grid vector. */
      vector_set(x, d, x1 + idx[d] * dx);

      /* check if the current dimension has overflowed. */
      if (idx[d] >= sz[d]) {
        vector_set(x, d, x1);
        idx[d] = 0;
      }
      else
        break;
    }
  }

  /* indicate successful completion. */
  status = 1;

fail:
  /* free all allocated memory and return. */
  vector_free(x);
  free(idx);
  free(sz);
  return status;
}

