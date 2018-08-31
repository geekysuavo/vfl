
/* include the gridding header. */
#include <vfl/util/grid.h>

/* grid_validate(): check that a pointer to a matrix structure
 * represents a valid grid.
 *
 * arguments:
 *  @grid: matrix of gridding information.
 *
 * returns:
 *  integer indicating whether (1) or not (0) the grid is valid.
 */
int grid_validate (const Matrix *grid) {
  /* valid grids have at least one row and exactly three columns. */
  return (grid && grid->rows >= 1 && grid->cols == 3);
}

/* grid_dims(): return the number of dimensions defined by a grid.
 *
 * arguments:
 *  @grid: matrix of gridding information.
 *
 * returns:
 *  number of grid dimensions.
 */
unsigned int grid_dims (const Matrix *grid) {
  /* return the grid row count. */
  return grid->rows;
}

/* grid_elements(): return the number of elements contained by a grid.
 *
 * arguments:
 *  @grid: matrix of gridding information.
 *
 * returns:
 *  number of elements implied by a gridding matrix.
 */
unsigned int grid_elements (const Matrix *grid) {
  /* compute the element count. */
  unsigned int elems = 0;
  grid_iterator_alloc(grid, &elems, NULL, NULL, NULL);

  /* return the computed count. */
  return elems;
}

/* grid_iterator_alloc(): prepare a set of grid traversal variables.
 *
 * arguments:
 *  @grid: matrix of gridding information.
 *  @elems: pointer to the output element count, or NULL.
 *  @idx: pointer to the output index array, or NULL.
 *  @sz: pointer to the output size array, or NULL.
 *  @x: pointer to the output grid location, or NULL.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int grid_iterator_alloc (const Matrix *grid,
                         unsigned int *elems,
                         unsigned int **idx,
                         unsigned int **sz,
                         Vector **x) {
  /* initialize the element count. */
  const unsigned int D = grid_dims(grid);
  unsigned int N = 1;

  /* initialize the results. */
  if (elems) *elems = 0;
  if (idx) *idx = NULL;
  if (sz) *sz = NULL;
  if (x) *x = NULL;

  /* check if the index should be allocated. */
  if (idx) {
    /* allocate the index array. */
    *idx = malloc(D * sizeof(unsigned int));
    if (!(*idx))
      return 0;
  }

  /* check if the size should be allocated. */
  if (sz) {
    /* allocate the array. */
    *sz = malloc(D * sizeof(unsigned int));
    if (!(*sz))
      return 0;
  }

  /* check if the grid vector should be allocated. */
  if (x) {
    /* allocate the vector. */
    *x = vector_alloc(D);
    if (!(*x))
      return 0;

    /* initialize the vector values. */
    matrix_copy_col(*x, grid, 0);
  }

  /* loop to determine the sizes and element count. */
  for (unsigned int d = 0; d < D; d++) {
    /* get the start, end, and step along this dimension. */
    const double x1 = matrix_get(grid, d, 0);
    const double x2 = matrix_get(grid, d, 2);
    const double dx = matrix_get(grid, d, 1);

    /* compute the size along this dimension. */
    const unsigned int szd = (unsigned int) floor((x2 - x1) / dx) + 1;
    N *= szd;

    /* store the index and size values, if necessary. */
    if (idx) (*idx)[d] = 0;
    if (sz) (*sz)[d] = szd;
  }

  /* store the element count and return success. */
  if (elems) *elems = N;
  return 1;
}

/* grid_iterator_free(): free variables allocated for grid traversal.
 *
 * arguments:
 *  @idx: index array.
 *  @sz: size array.
 *  @x: grid vector.
 */
void grid_iterator_free (unsigned int *idx,
                         unsigned int *sz,
                         Vector *x) {
  /* free the variables. */
  vector_free(x);
  free(idx);
  free(sz);
}

/* grid_iterator_next(): move to the next grid point during traversal.
 *
 * arguments:
 *  @grid: gridding matrix.
 *  @idx: index array.
 *  @sz: size array.
 *  @x: grid vector.
 *
 * returns:
 *  whether (1) or not (0) the grid has been fully traversed.
 */
int grid_iterator_next (const Matrix *grid,
                        unsigned int *idx,
                        unsigned int *sz,
                        Vector *x) {
  /* initialize the rollover flag. */
  const unsigned int D = grid_dims(grid);
  int roll = 0;

  /* loop over the grid dimensions. */
  for (unsigned int d = 0; d < D; d++) {
    /* get the start and step along this dimension. */
    const double x1 = matrix_get(grid, d, 0);
    const double dx = matrix_get(grid, d, 1);

    /* increment the index along this dimension. */
    idx[d]++;

    /* update the grid location vector. */
    vector_set(x, d, x1 + idx[d] * dx);

    /* check for overflow. */
    if (idx[d] >= sz[d]) {
      /* reset the vector and index. */
      vector_set(x, d, x1);
      idx[d] = 0;
    }
    else
      break;

    /* check for complete turnover. */
    if (d == D - 1)
      roll = 1;
  }

  /* return the logical inverse of the rollover flag. */
  return !roll;
}

