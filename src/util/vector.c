
/* include the vector header. */
#include <vfl/util/vector.h>

/* * * * inline function definitions: * * * */

/* vector_get(): get the value of a vector element.
 *
 * arguments:
 *  @v: vector to access.
 *  @i: element index.
 *
 * returns:
 *  value of the requested vector element.
 */
inline double vector_get (const Vector *v, size_t i) {
  /* return the element without bounds checking. */
  return v->data[i * v->stride];
}

/* vector_set(): set the value of a vector element.
 *
 * arguments:
 *  @v: vector to modify.
 *  @i: element index.
 *  @vi: new element value
 */
inline void vector_set (Vector *v, size_t i, double vi) {
  /* set the element without bounds checking. */
  v->data[i * v->stride] = vi;
}

/* * * * function definitions: * * * */

/* vector_bytes(): compute the space required for a vector.
 *
 * arguments:
 *  @len: number of elements in the vector.
 *
 * returns:
 *  number of bytes required for a vector structure having the
 *  specified number of elements.
 */
size_t vector_bytes (size_t len) {
  /* compute and return the space requirement. */
  return sizeof(Vector) + len * sizeof(double);
}

/* vector_init(): overlay a vector at a specified memory address.
 *
 * arguments:
 *  @addr: memory address to overlay structure onto.
 *  @len: number of elements in the overlaid vector.
 */
void vector_init (void *addr, size_t len) {
  /* cast the memory address to a vector structure pointer. */
  Vector *v = (Vector*) addr;

  /* store the structure parameters. */
  v->len = len;
  v->stride = 1;

  /* point the vector data array to the end of the structure. */
  v->data = (double*) ((char*) v + sizeof(Vector));
}

/* vector_alloc(): allocate a new vector for use.
 *
 * arguments:
 *  @len: number of elements to allocate to the vector.
 *
 * returns:
 *  newly allocated vector structure pointer. the elements of the
 *  vector will not yet be initialized.
 */
Vector *vector_alloc (size_t len) {
  /* allocate a new structure pointer, or fail. */
  const size_t bytes = vector_bytes(len);
  Vector *v = malloc(bytes);
  if (!v)
    return NULL;

  /* initialize and return the new structure pointer. */
  vector_init(v, len);
  return v;
}

/* vector_copy(): copy the contents of one vector into another of
 * identical size.
 *
 * arguments:
 *  @dest: destination vector structure pointer.
 *  @src: source vector structure pointer.
 */
void vector_copy (Vector *dest, const Vector *src) {
  /* copy each element without bounds checking. */
  for (size_t i = 0; i < dest->len; i++)
    vector_set(dest, i, vector_get(src, i));
}

/* vector_free(): free an allocated vector.
 *
 * arguments:
 *  @v: vector structure pointer to free.
 */
void vector_free (Vector *v) {
  /* return if the structure pointer is null. */
  if (!v) return;

  /* free the structure pointer (this includes the elements). */
  free(v);
}

/* vector_view_array(): create a vector view from a raw array of doubles.
 *
 * arguments:
 *  @data: data array to point into.
 *  @len: number of viewed elements.
 *
 * returns:
 *  newly created vector view.
 */
VectorView vector_view_array (double *data, size_t len) {
  /* initialize the view. */
  VectorView view = { 0, 0, NULL };

  /* the vector view will access each array element in order. */
  view.len = len;
  view.stride = 1;
  view.data = data;

  /* return the view. */
  return view;
}

/* vector_subvector(): create a vector view from a portion of
 * an existing vector.
 *
 * arguments:
 *  @v: original vector to point into.
 *  @offset: vector index of the first viewed element.
 *  @len: number of viewed elements.
 *
 * returns:
 *  newly created vector view.
 */
VectorView vector_subvector (const Vector *v, size_t offset, size_t len) {
  /* initialize the view. */
  VectorView view = { 0, 0, NULL };

  /* the view will access elements using the original vector's stride. */
  view.data = v->data + (offset * v->stride);
  view.stride = v->stride;
  view.len = len;

  /* return the view. */
  return view;
}

/* vector_max(): get the largest value of a vector.
 *
 * arguments:
 *  @v: vector to access.
 *
 * returns:
 *  largest element of the vector.
 */
double vector_max (const Vector *v) {
  /* identify the largest element of the vector. */
  double vmax = vector_get(v, 0);
  for (size_t i = 1; i < v->len; i++) {
    const double vi = vector_get(v, i);
    vmax = (vi > vmax ? vi : vmax);
  }

  /* return the identified value. */
  return vmax;
}

/* vector_set_all(): set all elements of a vector to a specified value.
 *
 * arguments:
 *  @v: vector to modify.
 *  @vall: new element value.
 */
void vector_set_all (Vector *v, double vall) {
  /* set all elements of the vector. */
  for (size_t i = 0; i < v->len; i++)
    vector_set(v, i, vall);
}

/* vector_set_zero(): set all elements of a vector to zero.
 *
 * arguments:
 *  @v: vector to modify.
 */
inline void vector_set_zero (Vector *v) {
  /* zero all elements of the vector. */
  vector_set_all(v, 0.0);
}

/* vector_add(): add one vector into another element-wise.
 *
 * operation:
 *  a <- a + b
 *
 * arguments:
 *  @a: first input and output vector.
 *  @b: second input vector.
 */
void vector_add (Vector *a, const Vector *b) {
  /* perform the element-wise sum. */
  for (size_t i = 0; i < a->len; i++)
    vector_set(a, i, vector_get(a, i) + vector_get(b, i));
}

/* vector_add_const(): add a constant value to a vector element-wise.
 *
 * operation:
 *  a <- a + beta
 *
 * arguments:
 *  @a: input and output vector.
 *  @beta: constant to add.
 */
void vector_add_const (Vector *v, double beta) {
  /* perform the element-wise sum. */
  for (size_t i = 0; i < v->len; i++)
    vector_set(v, i, vector_get(v, i) + beta);
}

/* vector_equal(): test if two vectors are element-wise equal.
 *
 * arguments:
 *  @a: first input vector.
 *  @b: second input vector.
 *
 * returns:
 *  integer indicating whether (1) or not (0) the vectors are equal.
 */
int vector_equal (const Vector *a, const Vector *b) {
  /* test each pair of elements for inequality. */
  for (size_t i = 0; i < a->len; i++) {
    if (vector_get(a, i) != vector_get(b, i))
      return 0;
  }

  /* all elements passed. the vectors are equal. */
  return 1;
}

/* vector_positive(): test if a vector is element-wise positive.
 *
 * arguments:
 *  @v: input vector to test.
 *
 * returns:
 *  integer indicating whether (1) or not (0) the vector is positive.
 */
int vector_positive (const Vector *v) {
  /* test each vector element for non-positivity. */
  for (size_t i = 0; i < v->len; i++) {
    if (vector_get(v, i) <= 0.0)
      return 0;
  }

  /* all elements passed. the vector is positive. */
  return 1;
}

/* vector_dispfn(): display the name and contents of a vector.
 *
 * arguments:
 *  @v: input vector structure pointer.
 *  @str: variable name of the vector.
 */
void vector_dispfn (const Vector *v, const char *str) {
  /* print the variable name. */
  printf("%s =\n", str);

  /* print the vector elements. */
  for (size_t i = 0; i < v->len; i++)
    printf("  %13.5lf\n", vector_get(v, i));
}

