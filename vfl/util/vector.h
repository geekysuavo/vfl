
/* ensure once-only inclusion. */
#ifndef __VFL_VECTOR_H__
#define __VFL_VECTOR_H__

/* include c library headers. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* vector_disp(): macro function for displaying the contents of vectors.
 */
#define vector_disp(v) vector_dispfn(v, #v)

/* Vector: structure for holding a real vector.
 */
typedef struct {
  /* data access parameters:
   *  @len: number of vector elements.
   *  @stride: array spacing between elements.
   */
  unsigned int len;
  unsigned int stride;

  /* @data: array or pointer to vector elements.
   */
  double *data;
}
Vector;

/* VectorView: aliased type used to explicitly note that a given
 * vector data structure does not own its data array.
 */
typedef Vector VectorView;

/* function declarations (util/vector.c): */

unsigned int vector_bytes (const unsigned int len);

void vector_init (void *addr, const unsigned int len);

Vector *vector_alloc (const unsigned int len);

void vector_copy (Vector *dest, const Vector *src);

void vector_free (Vector *v);

VectorView vector_view_array (double *data, const unsigned int len);

VectorView vector_subvector (const Vector *v,
                             const unsigned int offset,
                             const unsigned int len);

double vector_get (const Vector *v, const unsigned int i);

double vector_max (const Vector *v);

void vector_set (Vector *v, const unsigned int i, const double vi);

void vector_set_all (Vector *v, const double vall);

void vector_set_zero (Vector *v);

void vector_add (Vector *a, const Vector *b);

void vector_add_const (Vector *v, const double beta);

int vector_equal (const Vector *a, const Vector *b);

int vector_positive (const Vector *v);

void vector_dispfn (const Vector *v, const char *str);

#endif /* !__VFL_VECTOR_H__ */

