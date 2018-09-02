
/* include the gridding and dataset headers. */
#include <vfl/util/grid.h>
#include <vfl/vfl.h>

/* data_inner(): compute the inner product of the observations
 * stored within a dataset.
 *
 * warning: this function does not check the dataset structure
 * pointer for validity; use with caution!
 *
 * arguments:
 *  @dat: dataset structure pointer to access.
 *
 * returns:
 *  sum of all squared observation values.
 */
double data_inner (const Data *dat) {
  /* initialize the computation. */
  double yy = 0.0;

  /* compute the inner product. */
  Datum *di = dat->data;
  for (size_t i = 0; i < dat->N; i++, di++)
    yy += di->y * di->y;

  /* return the result. */
  return yy;
}

/* data_get(): extract an observation from a dataset.
 *
 * arguments:
 *  @dat: dataset structure pointer to access.
 *  @i: observation index to extract.
 *
 * returns:
 *  pointer to the requested observation, or NULL if the
 *  observation index is out of bounds.
 */
Datum *data_get (const Data *dat, size_t i) {
  /* check the input arguments. */
  if (!dat || i >= dat->N)
    return NULL;

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
int data_set (Data *dat, size_t i, const Datum *d) {
  /* check the input pointers. */
  if (!dat || !d || !d->x)
    return 0;

  /* check the index and dimensions. */
  if (i >= dat->N || d->x->len != dat->D)
    return 0;

  /* store the observation. */
  vector_copy(dat->data[i].x, d->x);
  dat->data[i].y = d->y;
  dat->data[i].p = d->p;

  /* return success. */
  return data_sort_single(dat, i);
}

/* data_find(): search for an observation in a dataset.
 *
 * arguments:
 *  @dat: dataset structure pointer to access.
 *  @i: observation index to extract.
 *  @d: pointer to the observation.
 *
 * returns:
 *  integer indicating whether (1) or not (0) the assignment succeeded.
 */
size_t data_find (const Data *dat, const Datum *d) {
  /* check the input pointers. */
  if (!dat || !d)
    return 0;

  /* return if the dataset is empty. */
  if (dat->N == 0)
    return 0;

  /* initialize the search bounds. */
  size_t imin = 0;
  size_t imax = dat->N - 1;

  /* search for the datum. */
  while (imin <= imax) {
    /* get the midpoint and compare. */
    const size_t i = (imin + imax) / 2;
    const int cmp = datum_cmp(dat->data + i, d);

    /* check the comparison result. */
    if (cmp == 0)
      return i + 1;
    else if (cmp < 0)
      imin = i + 1;
    else
      imax = i - 1;
  }

  /* return failure. */
  return 0;
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
int data_augment (Data *dat, const Datum *d) {
  /* check the input pointers. */
  if (!dat || !d || !d->x)
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
  return data_sort_single(dat, dat->N - 1);
}

/* data_augment_from_grid(): add a regular grid of zero-valued
 * observations into a dataset.
 *
 * arguments:
 *  @dat: dataset structure pointer to modify.
 *  @p: output index of the augmenting grid.
 *  @grid: matrix of gridding information.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int data_augment_from_grid (Data *dat, size_t p, const Matrix *grid) {
  /* declare required variables:
   *  @N0: previous dataset size.
   *  @N, @D: new dataset sizes.
   *  @idx: current grid index.
   *  @sz: maximum grid indices.
   *  @x: vector of grid locations.
   */
  size_t N0, N, D, *idx, *sz;
  int status;
  Vector *x;

  /* check the dataset and validate the gridding matrix. */
  if (!dat || !grid_validate(grid))
    return 0;

  /* initialize the new sizes. */
  D = grid_dims(grid);
  N0 = dat->N;

  /* check the dimensionality of the grid. */
  if (dat->D && dat->N && D != dat->D)
    return 0;

  /* initialize the return status. */
  status = 0;

  /* allocate memory for grid traversal. */
  if (!grid_iterator_alloc(grid, &N, &idx, &sz, &x))
    goto fail;

  /* attempt to resize the dataset. */
  if (!data_resize(dat, N0 + N, D))
    goto fail;

  /* loop over every grid point. */
  for (size_t i = 0; i < N; i++) {
    /* store the current grid point. */
    vector_copy(dat->data[N0 + i].x, x);
    dat->data[N0 + i].y = 0.0;
    dat->data[N0 + i].p = p;

    /* move to the next grid point. */
    grid_iterator_next(grid, idx, sz, x);
  }

  /* indicate successful completion. */
  status = data_sort(dat);

fail:
  /* free all allocated memory and return. */
  grid_iterator_free(idx, sz, x);
  return status;
}

/* data_augment_from_data(): add the contents of one dataset into
 * another dataset as new entries.
 *
 * arguments:
 *  @dat: dataset structure pointer to modify.
 *  @dsrc: source dataset to add into @dat.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int data_augment_from_data (Data *dat, const Data *dsrc) {
  /* check the input pointers. */
  if (!dat || !dsrc)
    return 0;

  /* initialize the new sizes. */
  const size_t D = dsrc->D;
  const size_t N = dsrc->N;
  const size_t N0 = dat->N;

  /* check the dimensionality of the source dataset. */
  if (dat->D && dat->N && D != dat->D)
    return 0;

  /* attempt to resize the dataset. */
  if (!data_resize(dat, N0 + N, D))
    return 0;

  /* loop over every augmenting point. */
  for (size_t i = 0; i < N; i++) {
    /* get the source and destination data. */
    Datum *di_src = data_get(dsrc, i);
    Datum *di = data_get(dat, N0 + i);

    /* copy from source to destination. */
    vector_copy(di->x, di_src->x);
    di->y = di_src->y;
    di->p = di_src->p;
  }

  /* return the result of sorting the augmented dataset. */
  return data_sort(dat);
}

