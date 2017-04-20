
/* include the gridding header. */
#include <vfl/util/grid.h>

/* FIXME: comment! */
int grid_validate (const matrix_t *grid) {
  return (grid && grid->rows >= 1 && grid->cols == 3);
}

unsigned int grid_dims (const matrix_t *grid) {
  return grid->rows;
}

unsigned int grid_elements (const matrix_t *grid) {
  unsigned int elems = 0;
  grid_iterator_alloc(grid, &elems, NULL, NULL, NULL);

  return elems;
}

int grid_iterator_alloc (const matrix_t *grid,
                         unsigned int *elems,
                         unsigned int **idx,
                         unsigned int **sz,
                         vector_t **x) {
  const unsigned int D = grid_dims(grid);
  unsigned int N = 1;

  if (idx) *idx = NULL;
  if (sz) *sz = NULL;
  if (x) *x = NULL;

  if (idx && sz) {
    *idx = malloc(D * sizeof(unsigned int));
    *sz = malloc(D * sizeof(unsigned int));

    if (!(*idx) || !(*sz))
      return 0;
  }

  if (x) {
    *x = vector_alloc(D);

    if (!(*x))
      return 0;

    matrix_copy_col(*x, grid, 0);
  }

  for (unsigned int d = 0; d < D; d++) {
    const double x1 = matrix_get(grid, d, 0);
    const double x2 = matrix_get(grid, d, 2);
    const double dx = matrix_get(grid, d, 1);

    const unsigned int szd = (unsigned int) floor((x2 - x1) / dx) + 1;
    N *= szd;

    if (idx) (*idx)[d] = 0;
    if (sz) (*sz)[d] = szd;
  }

  if (elems) *elems = N;
  return 1;
}

void grid_iterator_free (unsigned int *idx,
                         unsigned int *sz,
                         vector_t *x) {
  vector_free(x);
  free(idx);
  free(sz);
}

int grid_iterator_next (const matrix_t *grid,
                        unsigned int *idx,
                        unsigned int *sz,
                        vector_t *x) {
  const unsigned int D = grid_dims(grid);
  int roll = 0;

  for (unsigned int d = 0; d < D; d++) {
    const double x1 = matrix_get(grid, d, 0);
    const double dx = matrix_get(grid, d, 1);

    idx[d]++;

    vector_set(x, d, x1 + idx[d] * dx);

    if (idx[d] >= sz[d]) {
      vector_set(x, d, x1);
      idx[d] = 0;
    }
    else
      break;

    if (d == D - 1)
      roll = 1;
  }

  return !roll;
}

