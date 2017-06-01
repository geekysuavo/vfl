
/* ensure once-only inclusion. */
#ifndef __VFL_TIMER_H__
#define __VFL_TIMER_H__

/* include c library headers. */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/* include vfl headers. */
#include <vfl/base/object.h>

/* OBJECT_IS_TIMER(): check if an object is a timer.
 */
#define OBJECT_IS_TIMER(obj) \
  (OBJECT_TYPE(obj) == vfl_object_timer)

/* tmr_t: structure for holding a timer.
 */
typedef struct {
  /* base structure members. */
  OBJECT_BASE;

  /* timing structure members:
   *  @tick, @tock: start and stop times.
   *  @elapsed: cumulative elapsed time.
   *  @running: timer state.
   */
  struct timespec tick, tock;
  double elapsed;
  int running;
}
tmr_t;

/* function declarations (base/timer.c): */

#define timer_alloc() \
  (tmr_t*) obj_alloc(vfl_object_timer)

void timer_start (tmr_t *T);

void timer_stop (tmr_t *T);

void timer_reset (tmr_t *T);

/* available object types: */

extern const object_type_t *vfl_object_timer;

#endif /* !__VFL_TIMER_H__ */

