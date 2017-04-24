
/* include the dataset header. */
#include <vfl/data.h>

/* data_swap(): swap two data within a dataset.
 *
 * arguments:
 *  @dat: dataset structure pointer.
 *  @i: first element index.
 *  @j: second element index.
 */
static inline void data_swap (data_t *dat,
                              const unsigned int i,
                              const unsigned int j) {
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

/* data_cmp(): compare two data for equality or inequality.
 *
 * arguments:
 *  @d1: first datum structure pointer.
 *  @d2: second datum structure pointer.
 *
 * returns:
 *  -1,  if d1 < d2
 *  +1,  if d1 > d2
 *   0,  if d1 = d2
 */
int data_cmp (const datum_t *d1, const datum_t *d2) {
  /* examine output indices first. */
  if (d1->p < d2->p)
    return -1;
  else if (d1->p > d2->p)
    return 1;

  /* examine locations next. */
  const unsigned int D = d1->x->len;
  for (unsigned int d = 0; d < D; d++) {
    /* get the location vector elements. */
    const double x1 = vector_get(d1->x, d);
    const double x2 = vector_get(d2->x, d);

    /* compare the vector elements. */
    if (x1 < x2)
      return -1;
    else if (x1 > x2)
      return 1;
  }

  /* no differences detected, return equality. */
  return 0;
}

/* data_sort_repair(): repair the sub-tree rooted at a given index.
 *
 * arguments:
 *  @dat: dataset structure pointer.
 *  @i: index of the sub-tree to heapify.
 *  @n: total size of the heap.
 */
static void data_sort_repair (data_t *dat,
                              const unsigned int i,
                              const unsigned int n) {
  /* determine the left and right node indices. */
  unsigned int left = (2 * i) + 1;
  unsigned int right = (2 * i) + 2;
  unsigned int max = i;

  /* check if the left node has a larger value. */
  if (left < n && data_cmp(dat->data + left, dat->data + max) > 0)
    max = left;

  /* check if the right node has a value that is larger still. */
  if (right < n && data_cmp(dat->data + right, dat->data + max) > 0)
    max = right;

  /* check if any swaps are needed. */
  if (max != i) {
    /* swap and repair the sub-tree. */
    data_swap(dat, i, max);
    data_sort_repair(dat, max, n);
  }
}

/* data_sort_heapify(): array elements of a dataset into heap order.
 *
 * arguments:
 *  @dat: dataset structure pointer.
 *  @n: number of elements to heapify.
 */
static void data_sort_heapify (data_t *dat, const unsigned int n) {
  /* heapify each leaf node, working backwards up to the root. */
  for (long i = n / 2 - 1; i >= 0; i--)
    data_sort_repair(dat, i, n);
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
int data_sort (data_t *dat) {
  /* check the structure pointer. */
  if (!dat)
    return 0;

  /* return success if the dataset is empty. */
  if (dat->N == 0)
    return 1;

  /* re-arrange the datum elements to satisfy the heap property. */
  data_sort_heapify(dat, dat->N);

  /* pop all elements off the heap to obtain a sorted array. */
  unsigned int n = dat->N - 1;
  while (n > 0) {
    data_swap(dat, n, 0);
    data_sort_repair(dat, 0, --n);
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
int data_sort_single (data_t *dat, const unsigned int i) {
  /* check the input arguments. */
  if (!dat || i >= dat->N)
    return 0;

  /* initialize the sorting index. */
  const unsigned int jmax = dat->N - 1;
  unsigned int j = i;

  /* while the element is less than its leftmost neighbor... */
  while (j > 0 && data_cmp(dat->data + j, dat->data + (j - 1)) < 0) {
    /* ... shift it left. */
    data_swap(dat, j, j - 1);
    j--;
  }

  /* while the element is greater than its rightmost neighbor... */
  while (j < jmax && data_cmp(dat->data + j, dat->data + (j + 1)) > 0) {
    /* ... shift it right. */
    data_swap(dat, j, j + 1);
    j++;
  }

  /* return success. */
  return 1;
}

