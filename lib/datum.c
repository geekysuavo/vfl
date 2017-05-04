
/* include the datum header. */
#include <vfl/datum.h>

/* datum_cmp(): compare two data for equality or inequality.
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
int datum_cmp (const datum_t *d1, const datum_t *d2) {
  /* examine output indices first. */
  if (d1->p < d2->p)
    return -1;
  else if (d1->p > d2->p)
    return +1;

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
      return +1;
  }

  /* no differences detected, return equality. */
  return 0;
}

