
/* ensure once-only inclusion. */
#ifndef __VFL_VECTOR_H__
#define __VFL_VECTOR_H__

/* include c library headers. */
#include <stdio.h>
#include <stddef.h>
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
  size_t len;
  size_t stride;

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

size_t vector_bytes (size_t len);

void vector_init (void *addr, size_t len);

Vector *vector_alloc (size_t len);

void vector_copy (Vector *dest, const Vector *src);

void vector_free (Vector *v);

VectorView vector_view_array (double *data, size_t len);

VectorView vector_subvector (const Vector *v, size_t offset, size_t len);

double vector_get (const Vector *v, size_t i);

double vector_max (const Vector *v);

void vector_set (Vector *v, size_t i, double vi);

void vector_set_all (Vector *v, double vall);

void vector_set_zero (Vector *v);

void vector_add (Vector *a, const Vector *b);

void vector_add_const (Vector *v, double beta);

int vector_equal (const Vector *a, const Vector *b);

int vector_positive (const Vector *v);

void vector_dispfn (const Vector *v, const char *str);

#endif /* !__VFL_VECTOR_H__ */

