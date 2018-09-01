
/* include the vfl header. */
#include <vfl/vfl.h>

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
int data_resize (Data *dat, size_t N, size_t D) {
  /* determine the size of the observation array. */
  const size_t vbytes = vector_bytes(D);
  const size_t bytes = N * (sizeof(Datum) + vbytes);

  /* reallocate the swap vector. */
  Vector *swp = vector_alloc(D);
  if (!swp)
    return 0;

  /* allocate a new observation array. */
  Datum *data = malloc(bytes);
  if (!data)
    return 0;

  /* create a pointer for indexing datum elements. */
  char *ptr = (char*) data;
  ptr += N * sizeof(Datum);

  /* initialize each member of the observation array. */
  for (size_t i = 0; i < N; i++) {
    /* initialize the location vector. */
    data[i].x = (Vector*) ptr;
    vector_init(data[i].x, D);
    ptr += vbytes;
  }

  /* copy any existing observations into the new array. */
  for (size_t i = 0; i < dat->N; i++) {
    vector_copy(data[i].x, dat->data[i].x);
    data[i].y = dat->data[i].y;
    data[i].p = dat->data[i].p;
  }

  /* replace the swap vector. */
  vector_free(dat->swp.x);
  dat->swp.x = swp;

  /* replace the observation array. */
  free(dat->data);
  dat->data = data;

  /* store the new dataset sizes. */
  dat->N = N;
  dat->D = D;

  /* return success. */
  return 1;
}

