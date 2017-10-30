
/* ensure once-only inclusion. */
#ifndef __VFL_GRID_H__
#define __VFL_GRID_H__

/* include the matrix header. */
#include <vfl/util/matrix.h>

/* function declarations (util/grid.c): */

int grid_validate (const Matrix *grid);

unsigned int grid_dims (const Matrix *grid);

unsigned int grid_elements (const Matrix *grid);

int grid_iterator_alloc (const Matrix *grid,
                         unsigned int *elems,
                         unsigned int **idx,
                         unsigned int **sz,
                         Vector **x);

void grid_iterator_free (unsigned int *idx,
                         unsigned int *sz,
                         Vector *x);

int grid_iterator_next (const Matrix *grid,
                        unsigned int *idx,
                        unsigned int *sz,
                        Vector *x);

#endif /* !__VFL_GRID_H__ */

