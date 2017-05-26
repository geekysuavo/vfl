
/* include c library headers. */
#include <math.h>

/* include the math header. */
#include <vfl/base/math.h>
#include <vfl/base/map.h>
#include <vfl/base/list.h>
#include <vfl/base/float.h>

/* MATH_METHOD_UNARY_DEF(): macro function for defining a method
 * wrapping a unary function mapping one double to another.
 */
#define MATH_METHOD_UNARY_DEF(fn) \
static object_t *mathobj_ ## fn (const object_t *math, map_t *args) { \
  return mathobj_unary(args, fn); }

/* MATH_METHOD_UNARY(): macro function for inserting a unary math
 * method into an object methods array.
 */
#define MATH_METHOD_UNARY(fn) \
  { #fn, (object_method_fn) mathobj_ ## fn }

/* MATH_METHOD_UNARY_NAMED(): macro function for inserting a uniquely
 * named unary math method into an object methods array.
 */
#define MATH_METHOD_UNARY_NAMED(fn,name) \
  { #name, (object_method_fn) mathobj_ ## fn }

/* math_unary_fn(): compute a unary function.
 *
 * arguments:
 *  @x: function input.
 *
 * returns:
 *  function output value.
 */
typedef double (*math_unary_fn) (double x);

/* mathobj_unary(): compute the result of a unary function on one
 * of the following types:
 *  - scalar number: num (int|float)
 *  - vector:        [num]*int
 *  - matrix:        [[num]*int]*int
 *
 * arguments:
 *  @args: mapping holding function arguments.
 *  @func: unary c function pointer.
 *
 * returns:
 *  computed result, if the input argument specified by "v"
 *  holds an acceptable type.
 */
static object_t *mathobj_unary (map_t *args, math_unary_fn func) {
  /* get the input value. */
  object_t *obj = map_get(args, "v");
  if (!obj)
    return NULL;

  /* declare a variable to hold the output value. */
  object_t *val = NULL;

  /* determine the input type. */
  if (OBJECT_IS_NUM(obj)) {
    /* scalar number. */
    const double fval = func(num_get(obj));
    val = (object_t*) float_alloc_with_value(fval);
  }
  else if (OBJECT_IS_LIST(obj)) {
    /* vector or matrix. try vectors first. */
    vector_t *x = list_to_vector((list_t*) obj);
    if (x) {
      /* valid vector. compute the vector elements. */
      for (unsigned int i = 0; i < x->len; i++)
        vector_set(x, i, func(vector_get(x, i)));

      /* cast the vector back to a list. */
      val = list_alloc_from_vector(x);
      vector_free(x);
    }
    else {
      /* try matrix instead. */
      matrix_t *A = list_to_matrix((list_t*) obj);
      if (A) {
        /* valid matrix. compute the matrix elements. */
        for (unsigned int i = 0; i < A->rows; i++)
          for (unsigned int j = 0; j < A->cols; j++)
            matrix_set(A, i, j, func(matrix_get(A, i, j)));

        /* cast the matrix back to a list. */
        val = list_alloc_from_matrix(A);
        matrix_free(A);
      }
    }
  }

  /* return the computed value. */
  return val;
}

/* absolute values and roots. */
MATH_METHOD_UNARY_DEF (fabs)
MATH_METHOD_UNARY_DEF (sqrt)
MATH_METHOD_UNARY_DEF (cbrt)

/* log and exp. */
MATH_METHOD_UNARY_DEF (log)
MATH_METHOD_UNARY_DEF (log2)
MATH_METHOD_UNARY_DEF (log10)
MATH_METHOD_UNARY_DEF (exp)
MATH_METHOD_UNARY_DEF (exp2)

/* trig. */
MATH_METHOD_UNARY_DEF (sin)
MATH_METHOD_UNARY_DEF (cos)
MATH_METHOD_UNARY_DEF (tan)

/* hyperbolic trig. */
MATH_METHOD_UNARY_DEF (sinh)
MATH_METHOD_UNARY_DEF (cosh)
MATH_METHOD_UNARY_DEF (tanh)

/* inverse trig. */
MATH_METHOD_UNARY_DEF (asin)
MATH_METHOD_UNARY_DEF (acos)
MATH_METHOD_UNARY_DEF (atan)

/* inverse hyperbolic trig. */
MATH_METHOD_UNARY_DEF (asinh)
MATH_METHOD_UNARY_DEF (acosh)
MATH_METHOD_UNARY_DEF (atanh)

/* math_methods: array of callable math library methods.
 */
static object_method_t math_methods[] = {
  /* absolute values and roots. */
  MATH_METHOD_UNARY_NAMED (fabs, abs),
  MATH_METHOD_UNARY (sqrt),
  MATH_METHOD_UNARY (cbrt),

  /* log and exp. */
  MATH_METHOD_UNARY (log),
  MATH_METHOD_UNARY (log2),
  MATH_METHOD_UNARY (log10),
  MATH_METHOD_UNARY (exp),
  MATH_METHOD_UNARY (exp2),

  /* trig. */
  MATH_METHOD_UNARY (sin),
  MATH_METHOD_UNARY (cos),
  MATH_METHOD_UNARY (tan),

  /* hyperbolic trig. */
  MATH_METHOD_UNARY (sinh),
  MATH_METHOD_UNARY (cosh),
  MATH_METHOD_UNARY (tanh),

  /* inverse trig. */
  MATH_METHOD_UNARY (asin),
  MATH_METHOD_UNARY (acos),
  MATH_METHOD_UNARY (atan),

  /* inverse hyperbolic trig. */
  MATH_METHOD_UNARY (asinh),
  MATH_METHOD_UNARY (acosh),
  MATH_METHOD_UNARY (atanh),

  /* end marker. */
  { NULL, NULL }
};

/* math_type: math library type structure.
 */
static object_type_t math_type = {
  "math",                                        /* name      */
  sizeof(object_t),                              /* size      */

  NULL,                                          /* init      */
  NULL,                                          /* copy      */
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
  NULL,                                          /* props     */
  math_methods                                   /* methods   */
};

/* vfl_object_math: address of the math_type structure. */
const object_type_t *vfl_object_math = &math_type;

