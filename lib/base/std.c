
/* include c library headers. */
#include <stdio.h>

/* include the stdlib header. */
#include <vfl/base/std.h>
#include <vfl/base/map.h>
#include <vfl/base/list.h>
#include <vfl/base/int.h>
#include <vfl/base/float.h>
#include <vfl/base/string.h>

/* func std.disp (v: any);
 */
static object_t *stdobj_disp (object_t *std, map_t *args) {
  /* get the only argument. */
  object_t *arg = map_get(args, "v");
  if (arg) {
    /* get the argument type name. */
    const char *tname = OBJECT_TYPE(arg)->name;
    printf("%s(0x%lx)", tname, (size_t) arg);

    /* act based on the argument type. */
    if (OBJECT_IS_INT(arg))
      printf(": %ld", int_get((int_t*) arg));
    else if (OBJECT_IS_FLOAT(arg))
      printf(": %lg", float_get((flt_t*) arg));
    else if (OBJECT_IS_STRING(arg))
      printf(": '%s'", string_get((string_t*) arg));

    /* print a newline. */
    printf("\n");
  }

  /* return nothing. */
  VFL_RETURN_NIL;
}

/* func std.range (n: int);
 *                (start: int, end: int, step: 1);
 */
static object_t *stdobj_range (object_t *std, map_t *args) {
  /* declare required variables:
   *  @start: starting range value.
   *  @step: range step amount.
   *  @end: ending range value.
   *  @arg: mapped arguments.
   */
  long start, step, end;
  object_t *arg;

  /* initialize the range variables. */
  start = 0;
  step = 1;
  end = 0;

  /* get the range length. */
  arg = map_get(args, "n");
  if (arg) {
    if (OBJECT_IS_INT(arg))
      end = int_get((int_t*) arg) - 1;
    else
      return NULL;
  }

  /* get the start value. */
  arg = map_get(args, "start");
  if (arg) {
    if (OBJECT_IS_INT(arg))
      start = int_get((int_t*) arg);
    else
      return NULL;
  }

  /* get the end value. */
  arg = map_get(args, "end");
  if (arg) {
    if (OBJECT_IS_INT(arg))
      end = int_get((int_t*) arg);
    else
      return NULL;
  }

  /* get the step amount. */
  arg = map_get(args, "step");
  if (arg) {
    if (OBJECT_IS_INT(arg))
      step = int_get((int_t*) arg);
    else
      return NULL;
  }

  /* allocate a new list with appropriate length. */
  const size_t len = (size_t) floor((end - start) / step) + 1;
  list_t *lst = list_alloc_with_length(len);
  if (!lst)
    return NULL;

  /* set the list elements. */
  for (size_t i = 0; i < len; i++, start += step)
    list_set(lst, i, (object_t*) int_alloc_with_value(start));

  /* return the new list. */
  return (object_t*) lst;
}

/* std_methods: array of callable standard library methods.
 */
static object_method_t std_methods[] = {
  { "disp", (object_method_fn) stdobj_disp },
  { "range", (object_method_fn) stdobj_range },
  { NULL, NULL }
};

/* std_type: standard library type structure.
 */
static object_type_t std_type = {
  "std",                                         /* name      */
  sizeof(object_t),                              /* size      */

  NULL,                                          /* init      */
  NULL,                                          /* copy      */
  NULL,                                          /* free      */

  NULL,                                          /* add       */
  NULL,                                          /* sub       */
  NULL,                                          /* mul       */
  NULL,                                          /* div       */
  NULL,                                          /* pow       */

  NULL,                                          /* get       */
  NULL,                                          /* set       */
  NULL,                                          /* props     */
  std_methods                                    /* methods   */
};

/* vfl_object_std: address of the std_type structure. */
const object_type_t *vfl_object_std = &std_type;

