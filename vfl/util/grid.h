
/* ensure once-only inclusion. */
#ifndef __VFL_GRID_H__
#define __VFL_GRID_H__

/* include the matrix header. */
#include <vfl/util/matrix.h>

/* function declarations (grid.c): */

int grid_validate (const matrix_t *grid);

unsigned int grid_dims (const matrix_t *grid);

unsigned int grid_elements (const matrix_t *grid);

int grid_iterator_alloc (const matrix_t *grid,
                         unsigned int *elems,
                         unsigned int **idx,
                         unsigned int **sz,
                         vector_t **x);

void grid_iterator_free (unsigned int *idx,
                         unsigned int *sz,
                         vector_t *x);

int grid_iterator_next (const matrix_t *grid,
                        unsigned int *idx,
                        unsigned int *sz,
                        vector_t *x);

#endif /* !__VFL_GRID_H__ */

