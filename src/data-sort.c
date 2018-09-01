
/* include the vfl header. */
#include <vfl/vfl.h>

/* data_swap(): swap two data within a dataset.
 *
 * arguments:
 *  @dat: dataset structure pointer.
 *  @i: first element index.
 *  @j: second element index.
 */
static inline void data_swap (Data *dat, size_t i, size_t j) {
  /* copy the first element into the swap location. */
  vector_copy(dat->swp.x, dat->data[i].x);
  dat->swp.y = dat->data[i].y;
  dat->swp.p = dat->data[i].p;

  /* copy the second to the first. */
  vector_copy(dat->data[i].x, dat->data[j].x);
  dat->data[i].y = dat->data[j].y;
  dat->data[i].p = dat->data[j].p;

  /* copy the swap to the second. */
  vector_copy(dat->data[j].x, dat->swp.x);
  dat->data[j].y = dat->swp.y;
  dat->data[j].p = dat->swp.p;
}

/* data_sort(): sort the entries of a dataset.
 *
 * this function should never be necessary to call from the
 * outside world, as all dataset functions already maintain
 * sorted elements.
 *
 * arguments:
 *  @dat: dataset structure pointer.
 *
 * returns:
 *  integer indicating sort success (1) or failure (0).
 */
int data_sort (Data *dat) {
  /* check the structure pointer. */
  if (!dat)
    return 0;

  /* run an outer loop to sort every element. */
  for (size_t i = 0; i < dat->N; i++) {
    /* initialize the starting index. */
    size_t j = i;

    /* loop until the current element has been sorted. */
    while (j > 0 && datum_cmp(dat->data + (j - 1), dat->data + j) > 0) {
      data_swap(dat, j - 1, j);
      j--;
    }
  }

  /* return success. */
  return 1;
}

/* data_sort_single(): move a single entry within a dataset to
 * achieve a sorted state, assuming all other elements are in
 * sorted order.
 *
 * this function should never be necessary to call from the
 * outside world, as all dataset functions already maintain
 * sorted elements.
 *
 * arguments:
 *  @dat: dataset structure pointer.
 *  @i: dataset element index.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int data_sort_single (Data *dat, size_t i) {
  /* check the input arguments. */
  if (!dat || i >= dat->N)
    return 0;

  /* initialize the sorting index. */
  const size_t jmax = dat->N - 1;
  size_t j = i;

  /* while the element is less than its leftmost neighbor... */
  while (j > 0 && datum_cmp(dat->data + j, dat->data + (j - 1)) < 0) {
    /* ... shift it left. */
    data_swap(dat, j, j - 1);
    j--;
  }

  /* while the element is greater than its rightmost neighbor... */
  while (j < jmax && datum_cmp(dat->data + j, dat->data + (j + 1)) > 0) {
    /* ... shift it right. */
    data_swap(dat, j, j + 1);
    j++;
  }

  /* return success. */
  return 1;
}

