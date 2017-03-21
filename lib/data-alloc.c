
/* include the dataset header. */
#include <vfl/data.h>

/* data_alloc(): allocate a new empty dataset.
 *
 * returns:
 *  newly allocated and initialized dataset structure pointer.
 */
data_t *data_alloc (void) {
  /* allocate the structure pointer. */
  data_t *dat = (data_t*) malloc(sizeof(data_t));
  if (!dat)
    return NULL;

  /* initialize the size of the dataset. */
  dat->N = 0;
  dat->D = 0;

  /* initialize the data array. */
  dat->data = NULL;

  /* return the new dataset. */
  return dat;
}

/* data_alloc_from_file(): allocate a new dataset, filled with
 * observations that have been read from a file.
 *
 * arguments:
 *  @fname: filename to read observations from.
 *
 * returns:
 *  newly allocated and initialized dataset structure pointer.
 */
data_t *data_alloc_from_file (const char *fname) {
  /* allocate the structure pointer. */
  data_t *dat = data_alloc();
  if (!dat)
    return NULL;

  /* augment the dataset from a file. */
  if (!data_fread(dat, fname)) {
    /* free the dataset and return. */
    data_free(dat);
    return NULL;
  }

  /* return the new dataset. */
  return dat;
}

/* data_alloc_from_grid(): allocate a new dataset, filled with
 * a regular grid of zero-valued observations.
 *
 * arguments:
 *  @P: number of outputs to place grid points onto.
 *  @grid: matrix of gridding information.
 *
 * returns:
 *  newly allocated and initialized dataset structure pointer.
 */
data_t *data_alloc_from_grid (const unsigned int P,
                              const matrix_t *grid) {
  /* allocate the structure pointer. */
  data_t *dat = data_alloc();
  if (!dat)
    return NULL;

  /* create grids over all outputs. */
  for (unsigned int p = 0; p < P; p++) {
    /* augment the dataset with a grid. */
    if (!data_augment_from_grid(dat, p, grid)) {
      /* free the dataset and return. */
      data_free(dat);
      return NULL;
    }
  }

  /* return the new dataset. */
  return dat;
}

/* data_free(): free an allocated dataset.
 *
 * arguments:
 *  @dat: dataset structure pointer to free.
 */
void data_free (data_t *dat) {
  /* return if the structure pointer is null. */
  if (!dat)
    return;

  /* free the array of observations and the structure pointer. */
  free(dat->data);
  free(dat);
}

/* data_resize(): resize the observation array of a dataset.
 *
 * arguments:
 *  @dat: dataset structure pointer to modify.
 *  @N: observation count of the new dataset.
 *  @D: dimensionality of the new dataset.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int data_resize (data_t *dat, const unsigned int N, const unsigned int D) {
  /* determine the size of the observation array. */
  const unsigned int vbytes = vector_bytes(D);
  const unsigned int bytes = N * (sizeof(datum_t) + vbytes);

  /* allocate a new observation array. */
  datum_t *data = (datum_t*) malloc(bytes);
  if (!data)
    return 0;

  /* create a pointer for indexing datum elements. */
  char *ptr = (char*) data;
  ptr += N * sizeof(datum_t);

  /* initialize each vector in the observation array. */
  for (unsigned int i = 0; i < N; i++) {
    data[i].x = (vector_t*) ptr;
    vector_init(data[i].x, D);
    ptr += vbytes;
  }

  /* copy any existing observations into the new array. */
  for (unsigned int i = 0; i < dat->N; i++) {
    vector_copy(data[i].x, dat->data[i].x);
    data[i].y = dat->data[i].y;
    data[i].p = dat->data[i].p;
  }

  /* replace the observation array. */
  free(dat->data);
  dat->data = data;

  /* store the new dataset sizes. */
  dat->N = N;
  dat->D = D;

  /* return success. */
  return 1;
}

