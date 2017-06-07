
/* include the required object headers. */
#include <vfl/base/timer.h>
#include <vfl/base/float.h>
#include <vfl/base/map.h>

/* TIMER_PROP_DEF(): macro function for defining a scaled
 * timer property getter.
 */
#define TIMER_PROP_DEF(fn, sf) \
static object_t *timer_getprop_ ## fn (const tmr_t *T) { \
  return timer_getscaled(T, sf); }

/* TIMER_PROP(): macro function for inserting a scaled
 * timer property getter into an object properties array.
 */
#define TIMER_PROP(fn, sf) \
  { #fn, (object_getprop_fn) timer_getprop_ ## fn, NULL }

/* timer_init(): initialize a timer.
 *
 * arguments:
 *  @T: timer structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int timer_init (tmr_t *T) {
  /* restart the timer and return success. */
  timer_reset(T);
  timer_start(T);
  return 1;
}

/* timer_copy(): copy the contents of a timer.
 *
 * arguments:
 *  @T: source timer structure pointer.
 *  @Tdup: destination timer structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int timer_copy (const tmr_t *T, tmr_t *Tdup) {
  /* copy the timer value and return success. */
  Tdup->tick = T->tick;
  Tdup->tock = T->tock;
  Tdup->elapsed = T->elapsed;
  Tdup->running = T->running;
  return 1;
}

/* timer_start(): start a timer.
 *
 * arguments:
 *  @T: timer structure pointer.
 */
void timer_start (tmr_t *T) {
  /* return if the timer is already running. */
  if (T->running)
    return;

  /* start timing a new span. */
  clock_gettime(CLOCK_REALTIME, &T->tick);
  T->running = 1;
}

/* timer_stop(): stop a timer.
 *
 * arguments:
 *  @T: timer structure pointer.
 */
void timer_stop (tmr_t *T) {
  /* return if the timer is already stopped */
  if (!T->running)
    return;

  /* stop timing the current span. */
  clock_gettime(CLOCK_REALTIME, &T->tock);
  T->running = 0;

  /* compute the time difference. */
  struct timespec delta;
  if (T->tock.tv_nsec < T->tick.tv_nsec) {
    /* nanosecond overflow. */
    delta.tv_sec = T->tock.tv_sec - T->tick.tv_sec - 1;
    delta.tv_nsec = 1000000000 + T->tock.tv_nsec - T->tick.tv_nsec;
  }
  else {
    /* no nanosecond overflow. */
    delta.tv_sec = T->tock.tv_sec - T->tick.tv_sec;
    delta.tv_nsec = T->tock.tv_nsec - T->tick.tv_nsec;
  }

  /* update the elapsed time. */
  T->elapsed += 1.0e9 * (double) delta.tv_sec + (double) delta.tv_nsec;
}

/* timer_reset(): reset a timer.
 *
 * arguments:
 *  @T: timer structure pointer.
 */
void timer_reset (tmr_t *T) {
  /* reset the tick value. */
  T->tick.tv_sec = 0;
  T->tick.tv_nsec = 0;

  /* reset the tock value. */
  T->tock.tv_sec = 0;
  T->tock.tv_nsec = 0;

  /* reset the elapsed time. */
  T->elapsed = 0.0;

  /* stop the timer. */
  T->running = 0;
}

/* --- */

/* timer_getscaled(): get the unit-scaled elapsed time of a timer.
 *
 * arguments:
 *  @T: timer structure pointer to access.
 *  @sf: scale factor to multiply by.
 *
 * returns:
 *  object holding the scaled elapsed time.
 */
static object_t *timer_getscaled (const tmr_t *T, const double sf) {
  /* return the elapsed time multiplied by the scale factor. */
  return (object_t*) float_alloc_with_value(T->elapsed * sf);
}

/* timer_getprop_elapsed(): get the elapsed time of a timer.
 *  - see object_getprop_fn() for details.
 */
static object_t *timer_getprop_elapsed (const tmr_t *T) {
  /* return the elapsed time in nanoseconds. */
  return timer_getscaled(T, 1.0);
}

/* timer_getprop_running(): get the running state of a timer.
 *  - see object_getprop_fn() for details.
 */
static object_t *timer_getprop_running (const tmr_t *T) {
  /* return the running flag of the timer. */
  return (object_t*) int_alloc_with_value(T->running);
}

/* scaled elapsed time properties. */
TIMER_PROP_DEF (s,  1.0e-9)
TIMER_PROP_DEF (ms, 1.0e-6)
TIMER_PROP_DEF (us, 1.0e-3)
TIMER_PROP_DEF (ns, 1.0)

/* timer_properties: array of accessible object properties.
 */
static object_property_t timer_properties[] = {
  { "elapsed", (object_getprop_fn) timer_getprop_elapsed, NULL },
  { "running", (object_getprop_fn) timer_getprop_running, NULL },
  TIMER_PROP (s,  1.0e-9),
  TIMER_PROP (ms, 1.0e-6),
  TIMER_PROP (us, 1.0e-3),
  TIMER_PROP (ns, 1.0),
  { NULL, NULL, NULL }
};

/* --- */

/* timer_method_start(): timer start method.
 *  - see object_method_fn() for details.
 */
static object_t *timer_method_start (tmr_t *T, map_t *args) {
  /* start the timer and return nothing. */
  timer_start(T);
  VFL_RETURN_NIL;
}

/* timer_method_stop(): timer stop method.
 *  - see object_method_fn() for details.
 */
static object_t *timer_method_stop (tmr_t *T, map_t *args) {
  /* stop the timer and return nothing. */
  timer_stop(T);
  VFL_RETURN_NIL;
}

/* timer_method_reset(): timer reset method.
 *  - see object_method_fn() for details.
 */
static object_t *timer_method_reset (tmr_t *T, map_t *args) {
  /* reset the timer and return nothing. */
  timer_reset(T);
  VFL_RETURN_NIL;
}

/* timer_method_restart(): timer restart method.
 *  - see object_method_fn() for details.
 */
static object_t *timer_method_restart (tmr_t *T, map_t *args) {
  /* reset the timer and return nothing. */
  timer_reset(T);
  timer_start(T);
  VFL_RETURN_NIL;
}

/* timer_method_report(): timer elapsed time reporting method.
 *  - see object_method_fn() for details.
 */
static object_t *timer_method_report (tmr_t *T, map_t *args) {
  /* if the timer is running, pause to compute the elapsed time. */
  const int paused = T->running;
  if (paused)
    timer_stop(T);

  /* report the elapsed time. */
  printf("Elapsed time: %lg seconds.\n", T->elapsed * 1.0e-9);

  /* check if a reset was requested. */
  object_t *arg = map_get(args, "reset");
  if (arg && obj_test(arg))
    timer_reset(T);

  /* if the timer was running, restart it. */
  if (paused)
    timer_start(T);

  /* return nothing. */
  VFL_RETURN_NIL;
}

/* timer_methods: array of callable object methods.
 */
static object_method_t timer_methods[] = {
  { "start",   (object_method_fn) timer_method_start },
  { "stop",    (object_method_fn) timer_method_stop },
  { "reset",   (object_method_fn) timer_method_reset },
  { "restart", (object_method_fn) timer_method_restart },
  { "report",  (object_method_fn) timer_method_report },
  { NULL, NULL }
};

/* --- */

/* timer_type: timer type structure.
 */
static object_type_t timer_type = {
  "timer",                                       /* name      */
  sizeof(tmr_t),                                 /* size      */

  (object_init_fn) timer_init,                   /* init      */
  (object_copy_fn) timer_copy,                   /* copy      */
  NULL,                                          /* free      */
  NULL,                                          /* test      */
  NULL,                                          /* cmp       */

  NULL,                                          /* add       */
  NULL,                                          /* sub       */
  NULL,                                          /* mul       */
  NULL,                                          /* div       */
  NULL,                                          /* pow       */

  NULL,                                          /* get       */
  NULL,                                          /* set       */
  timer_properties,                              /* props     */
  timer_methods                                  /* methods   */
};

/* vfl_object_timer: address of the timer_type structure. */
const object_type_t *vfl_object_timer = &timer_type;

