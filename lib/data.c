
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

  /* initialize the data arrays. */
  dat->X = NULL;
  dat->y = NULL;

  /* return the new dataset. */
  return dat;
}

/* data_alloc_from_grid(): allocate a new dataset, filled with
 * a regular grid of zero-valued observations.
 *
 * arguments:
 *  @grid: matrix of gridding information.
 *
 * returns:
 *  newly allocated and initialized dataset structure pointer.
 */
data_t *data_alloc_from_grid (const matrix_t *grid) {
  /* allocate the structure pointer. */
  data_t *dat = data_alloc();
  if (!dat)
    return NULL;

  /* augment the dataset with a grid. */
  if (!data_augment_from_grid(dat, grid)) {
    /* free the dataset and return. */
    data_free(dat);
    return NULL;
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

  /* if allocated, free the data arrays. */
  if (dat->N) {
    free(dat->X);
    free(dat->y);
  }

  /* free the dataset structure pointer. */
  free(dat);
}

/* data_get(): extract an observation from a dataset.
 *
 * arguments:
 *  @dat: dataset structure pointer to access.
 *  @i: observation index to extract.
 *  @x: output observation vector.
 *  @y: output observed value.
 *
 * returns:
 *  integer indicating whether (1) or not (0) the assignment succeeded.
 */
int data_get (const data_t *dat, const unsigned int i,
              vector_t *x, double *y) {
  /* check the input pointers. */
  if (!dat || !x || !y)
    return 0;

  /* check the index and dimensions. */
  if (i >= dat->N || x->len != dat->D)
    return 0;

  /* extract the observation. */
  matrix_copy_row(x, dat->X, i);
  *y = vector_get(dat->y, i);

  /* return success. */
  return 1;
}

/* data_set(): store an observation into a dataset.
 *
 * arguments:
 *  @dat: dataset structure pointer to access.
 *  @i: observation index to extract.
 *  @x: input observation vector.
 *  @y: input observed value.
 *
 * returns:
 *  integer indicating whether (1) or not (0) the assignment succeeded.
 */
int data_set (data_t *dat, const unsigned int i,
              const vector_t *x, const double y) {
  /* check the input pointers. */
  if (!dat || !x)
    return 0;

  /* check the index and dimensions. */
  if (i >= dat->N || x->len != dat->D)
    return 0;

  /* store the observation vector. */
  vector_view_t xi = matrix_row(dat->X, i);
  vector_copy(&xi, x);

  /* store the observed value. */
  vector_set(dat->y, i, y);

  /* return success. */
  return 1;
}

/* data_augment(): add a new observation into a dataset.
 *
 * arguments:
 *  @dat: dataset structure pointer to access.
 *  @x: augmenting observation vector.
 *  @y: augmenting observed value.
 *
 * returns:
 *  integer indicating whether (1) or not (0) the augmentation succeeded.
 */
int data_augment (data_t *dat, const vector_t *x, const double y) {
  /* declare required variables:
   *  @N, @D: new observation and dimension counts.
   *  @Xnew: new observation matrix.
   *  @ynew: new observed vector.
   */
  unsigned int N, D;
  matrix_t *Xnew;
  vector_t *ynew;

  /* check the input pointers. */
  if (!dat || !x)
    return 0;

  /* check the observation dimensionality. */
  if (dat->D && dat->N && x->len != dat->D)
    return 0;

  /* determine the new dataset sizes. */
  N = dat->N + 1;
  D = x->len;

  /* allocate a new observation matrix. */
  Xnew = matrix_alloc(N, D);
  if (!Xnew)
    return 0;

  /* allocate a new vector of observed values. */
  ynew = vector_alloc(N);
  if (!ynew) {
    matrix_free(Xnew);
    return 0;
  }

  /* copy the existing data into the new data arrays. */
  for (unsigned int i = 0; i < N - 1; i++) {
    /* copy the observation matrix. */
    for (unsigned int d = 0; d < D; d++)
      matrix_set(Xnew, i, d, matrix_get(dat->X, i, d));

    /* copy the observed values. */
    vector_set(ynew, i, vector_get(dat->y, i));
  }

  /* store the augmenting observation. */
  for (unsigned int d = 0; d < D; d++)
    matrix_set(Xnew, N - 1, d, vector_get(x, d));

  /* store the augmenting observed value. */
  vector_set(ynew, N - 1, y);

  /* free the existing data arrays. */
  matrix_free(dat->X);
  vector_free(dat->y);

  /* store the new structure members. */
  dat->X = Xnew;
  dat->y = ynew;
  dat->N = N;
  dat->D = D;

  /* return success. */
  return 1;
}

/* augment_from_grid(): recursive function used by data_alloc_from_grid()
 * to augment a dataset with a regular grid of observations.
 *
 * arguments:
 *  @dat: dataset structure pointer to access.
 *  @grid: matrix of gridding information.
 *  @x: vector of observation values.
 *  @d: current recursion dimension.
 *
 * returns:
 *  integer indicating augmentation success (1) or failure (0).
 */
static int augment_from_grid (data_t *dat, const matrix_t *grid,
                              vector_t *x, const unsigned int d) {
  /* initialize the current dimension. */
  vector_set(x, d, matrix_get(grid, d, 0));

  /* loop over the current dimension values. */
  while (vector_get(x, d) <= matrix_get(grid, d, 2)) {
    /* either augment or recurse. */
    if (d == x->len - 1) {
      /* current dimension is last => augment. */
      if (!data_augment(dat, x, 0.0))
        return 0;
    }
    else {
      /* current dimension is not last => recurse. */
      if (!augment_from_grid(dat, grid, x, d + 1))
        return 0;
    }

    /* increment the current dimension. */
    vector_set(x, d, vector_get(x, d) + matrix_get(grid, d, 1));
  }

  /* return success. */
  return 1;
}

/* data_augment_from_grid(): add a regular grid of zero-valued
 * observations into a dataset.
 *
 * arguments:
 *  @dat: dataset structure pointer to access.
 *  @grid: matrix of gridding information.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int data_augment_from_grid (data_t *dat, const matrix_t *grid) {
  /* check the input pointers. */
  if (!dat || !grid)
    return 0;

  /* check the gridding matrix. */
  if (grid->cols != 3)
    return 0;

  /* allocate a vector for holding observations. */
  const unsigned int D = grid->rows;
  vector_t *x = vector_alloc(D);
  if (!x)
    return 0;

  /* recursively augment the dataset. */
  if (!augment_from_grid(dat, grid, x, 0)) {
    /* free the observation vector and return failure. */
    vector_free(x);
    return 0;
  }

  /* free the observation vector and return success. */
  vector_free(x);
  return 1;
}

/* data_fread(): read a text file into an allocated dataset structure.
 *
 * arguments:
 *  @dat: dataset structure pointer to augment.
 *  @fname: filename to read from.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int data_fread (data_t *dat, const char *fname) {
  /* check the input pointers. */
  if (!dat || !fname)
    return 0;

  /* open the input file. */
  FILE *fh = fopen(fname, "r");
  if (!fh)
    return 0;

  /* initialize temporary variables for reading the input file. */
  unsigned int ntok = 0;
  char buf[1024];

  /* count the number of fields in the first line. */
  if (fgets(buf, 1024, fh)) {
    char *tok = strtok(buf, " ");
    while (tok) {
      tok = strtok(NULL, " ");
      ntok++;
    }
  }

  /* check that the dataset has at least two columns. */
  if (ntok < 2) {
    fclose(fh);
    return 0;
  }

  /* allocate a temporary vector. */
  vector_t *x = vector_alloc(ntok - 1);
  double y;

  /* rewind and read the complete file. */
  fseek(fh, SEEK_SET, 0);
  while (!feof(fh)) {
    /* read a new line of the file. */
    if (fgets(buf, 1024, fh)) {
      /* read each observation input value. */
      char *tok = strtok(buf, " ");
      for (unsigned int d = 0; d < ntok - 1; d++) {
        vector_set(x, d, atof(tok));
        tok = strtok(NULL, " ");
      }

      /* read the observed value. */
      y = atof(tok);

      /* augment the dataset with the current observation. */
      if (!data_augment(dat, x, y)) {
        vector_free(x);
        fclose(fh);
        return 0;
      }
    }
  }

  /* free the temporary vector, close the input file and return success. */
  vector_free(x);
  fclose(fh);
  return 1;
}

/* data_fwrite(): write the contents of a dataset to a text file.
 *
 * arguments:
 *  @dat: dataset structure pointer to access.
 *  @fname: filename to write to.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int data_fwrite (const data_t *dat, const char *fname) {
  /* check the input pointers. */
  if (!dat || !fname)
    return 0;

  /* open the output file. */
  FILE *fh = fopen(fname, "w");
  if (!fh)
    return 0;

  /* loop over each observation. */
  for (unsigned int i = 0; i < dat->N; i++) {
    for (unsigned int d = 0; d < dat->D; d++)
      fprintf(fh, "%s%le", d == 0 ? "" : " ", matrix_get(dat->X, i, d));

    fprintf(fh, " %le\n", vector_get(dat->y, i));
  }

  /* close the output file and return success. */
  fclose(fh);
  return 1;
}

