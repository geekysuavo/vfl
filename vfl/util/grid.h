
/* ensure once-only inclusion. */
#ifndef __VFL_GRID_H__
#define __VFL_GRID_H__

/* include the matrix header. */
#include <vfl/util/matrix.h>

/* function declarations (util/grid.c): */

int grid_validate (const Matrix *grid);

size_t grid_dims (const Matrix *grid);

size_t grid_elements (const Matrix *grid);

int grid_iterator_alloc (const Matrix *grid,
                         size_t *elems,
                         size_t **idx,
                         size_t **sz,
                         Vector **x);

void grid_iterator_free (size_t *idx, size_t *sz, Vector *x);

int grid_iterator_next (const Matrix *grid, size_t *idx, size_t *sz,
                        Vector *x);

#endif /* !__VFL_GRID_H__ */

