
/* ensure once-only inclusion. */
#ifndef __VFL_FACTOR_H__
#define __VFL_FACTOR_H__

/* include vfl headers. */
#include <vfl/util/specfun.h>
#include <vfl/util/blas.h>
#include <vfl/data.h>

/* Factor_Check(): macro to check if a PyObject is a Factor.
 */
#define Factor_Check(v) PyObject_TypeCheck(v, &Factor_Type)

/* Factor_CheckExact(): macro to check if a PyObject is
 * precisely a Factor.
 */
#define Factor_CheckExact(v) (Py_TYPE(v) == &Factor_Type)

/* macros to blindly access the dimension, parameter and weight counts
 * of Factor objects.
 */
#define Factor_MAX_DIM(v)     (((Factor*) (v))->d + ((Factor*) (v))->D)
#define Factor_GET_DIMS(v)    (((Factor*) (v))->D)
#define Factor_GET_PARMS(v)   (((Factor*) (v))->P)
#define Factor_GET_WEIGHTS(v) (((Factor*) (v))->K)

/* Factor_Type: globally available factor type structure.
 */
PyAPI_DATA(PyTypeObject) Factor_Type;

/* Factor: defined type for the factor structure. */
typedef struct factor Factor;

/* factor_mean_fn(): return the a first-order mode or moment
 * of a basis element.
 *
 * arguments:
 *  @f: factor structure pointer.
 *  @x: observation input vector.
 *  @p: function output index.
 *  @i: basis element index.
 *
 * returns:
 *  E[ phi^{p}(x | theta(f))_i ]
 */
typedef double (*factor_mean_fn) (const Factor *f, const Vector *x,
                                  size_t p, size_t i);

/* factor_var_fn(): return the second moment of basis elements.
 *
 * arguments:
 *  @f: factor structure pointer.
 *  @x: observation input vector.
 *  @p: function output index.
 *  @i: first basis element index.
 *  @j: second basis element index.
 *
 * returns:
 *  E[ phi^{p}(x | theta(f))_i phi^{p}(x | theta(f))_j ]
 */
typedef double (*factor_var_fn) (const Factor *f, const Vector *x,
                                 size_t p, size_t i, size_t j);

/* factor_cov_fn(): return the covariance of basis elements.
 *
 * arguments:
 *  @f: factor structure pointer.
 *  @x1: first observation input vector.
 *  @x2: second observation input vector.
 *  @p1: first function output index.
 *  @p2: second function output index.
 *
 * returns:
 *  E[ phi^{p1}(x1 | theta(f)) phi^{p2}(x2 | theta(f)) ]
 */
typedef double (*factor_cov_fn) (const Factor *f,
                                 const Vector *x1,
                                 const Vector *x2,
                                 size_t p1, size_t p2);

/* factor_diff_mean_fn(): return the gradient of the first moment
 * of a basis element.
 *
 * arguments:
 *  @f: factor structure pointer.
 *  @x: observation input vector.
 *  @p: function output index.
 *  @i: basis element index.
 *  @df: vector of gradients.
 *
 * returns:
 *  grad(lambda) E[ phi^{p}(x | theta(f))_i ]
 */
typedef void (*factor_diff_mean_fn) (const Factor *f, const Vector *x,
                                     size_t p, size_t i, Vector *df);

/* factor_diff_var_fn(): return the gradient of the second moment
 * of a basis element.
 *
 * arguments:
 *  @f: factor structure pointer.
 *  @x: observation input vector.
 *  @p: function output index.
 *  @i: first basis element index.
 *  @j: second basis element index.
 *  @df: vector of gradients.
 *
 * returns:
 *  grad(lambda) E[ phi^{p}(x | theta(f))_i phi^{p}(x | theta(f))_j ]
 */
typedef void (*factor_diff_var_fn) (const Factor *f, const Vector *x,
                                    size_t p, size_t i, size_t j,
                                    Vector *df);

/* factor_meanfield_fn(): perform an assumed-density mean-field update
 * of a factor, given its associated prior factor and a set of required
 * coefficients.
 *
 * if all arguments except the factor structure pointer are null,
 * then the factor shall interpret the function call as a request
 * to prepare itself for receiving data.
 *
 * if all arguments except the factor and its prior are null,
 * then the factor shall interpret the function call as a
 * request to finalize its mean-field update.
 *
 * arguments:
 *  @f: factor structure pointer.
 *  @fp: prior factor structure pointer.
 *  @dat: observed data for updates.
 *  @b: vector of first-order coefficients.
 *  @B: matrix of second-order coefficients.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
typedef int (*factor_meanfield_fn) (Factor *f, const Factor *fp,
                                    const Datum *dat,
                                    Vector *b,
                                    Matrix *B);

/* factor_div_fn(): return the kullback-liebler divergence between
 * two factors of the same type, but different parameters.
 *
 * arguments:
 *  @f: first factor structure pointer.
 *  @f2: second factor structure pointer.
 *
 * returns:
 *  KL[ q(theta(f)) || q(theta(f2)) ]
 */
typedef double (*factor_div_fn) (const Factor *f, const Factor *f2);

/* factor_init_fn(): initialize a factor structure
 * in a type-specific manner.
 *
 * arguments:
 *  @f: factor structure pointer to initialize.
 *
 * returns:
 *  integer indicating initialization success (1) or failure (0).
 */
typedef int (*factor_init_fn) (Factor *f);

/* factor_resize_fn(): resize a factor structure.
 *
 * arguments:
 *  @f: factor structure pointer to modify.
 *  @D, @P, @K: new factor sizes.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
typedef int (*factor_resize_fn) (Factor *f, size_t D, size_t P, size_t K);

/* factor_kernel_fn(): write the kernel code of a factor structure.
 *
 * arguments:
 *  @f: factor structure pointer.
 *  @p0: parameter vector offset.
 *
 * returns:
 *  newly allocated string that contains the kernel code, or NULL
 *  on failure. the calling function is responsible for freeing
 *  the string after use.
 */
typedef char* (*factor_kernel_fn) (const Factor *f, size_t p0);

/* factor_set_fn(): assign a variational parameter in a factor.
 *
 * arguments:
 *  @f: factor structure pointer to modify.
 *  @i: parameter index to assign.
 *  @value: parameter value.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
typedef int (*factor_set_fn) (Factor *f, size_t i, double value);

/* factor_copy_fn(): copy any extra (e.g. aliased) memory from one
 * factor into another.
 *
 * arguments:
 *  @f: factor structure pointer to access.
 *  @fdup: factor structure pointer to copy into.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
typedef int (*factor_copy_fn) (const Factor *f, Factor *fdup);

/* factor_free_fn(): free any extra (e.g. aliased) memory that is
 * associated with a factor.
 *
 * arguments:
 *  @f: factor structure pointer to free.
 */
typedef void (*factor_free_fn) (Factor *f);

/* FACTOR_EVAL(): macro function for declaring and defining
 * functions conforming to factor_mean_fn() for evaluation.
 */
#define FACTOR_EVAL(name) \
double name ## _eval (const Factor *f, const Vector *x, \
                      size_t p, size_t i)

/* FACTOR_MEAN(): macro function for declaring and defining
 * functions conforming to factor_mean_fn() for expectations.
 */
#define FACTOR_MEAN(name) \
double name ## _mean (const Factor *f, const Vector *x, \
                      size_t p, size_t i)

/* FACTOR_VAR(): macro function for declaring and defining
 * functions conforming to factor_var_fn().
 */
#define FACTOR_VAR(name) \
double name ## _var (const Factor *f, const Vector *x, \
                     size_t p, size_t i, size_t j)

/* FACTOR_COV(): macro function for declaring and defining
 * functions conforming to factor_cov_fn().
 */
#define FACTOR_COV(name) \
double name ## _cov (const Factor *f, \
                     const Vector *x1, const Vector *x2, \
                     size_t p1, size_t p2)

/* FACTOR_DIFF_MEAN(): macro function for declaring and defining
 * functions conforming to factor_diff_mean_fn().
 */
#define FACTOR_DIFF_MEAN(name) \
void name ## _diff_mean (const Factor *f, const Vector *x, \
                         size_t p, size_t i, Vector *df)

/* FACTOR_DIFF_VAR(): macro function for declaring and defining
 * functions conforming to factor_diff_var_fn().
 */
#define FACTOR_DIFF_VAR(name) \
void name ## _diff_var (const Factor *f, const Vector *x, \
                        size_t p, size_t i, size_t j, \
                        Vector *df)

/* FACTOR_MEANFIELD(): macro function for declaring and defining
 * functions conforming to factor_meanfield_fn().
 */
#define FACTOR_MEANFIELD(name) \
int name ## _meanfield (Factor *f, const Factor *fp, \
                        const Datum *dat, \
                        Vector *b, \
                        Matrix *B)

/* macro functions for handling the specific types of mean-field calls.
 */
#define FACTOR_MEANFIELD_INIT  !fp && !dat
#define FACTOR_MEANFIELD_END    fp && !dat

/* FACTOR_DIV(): macro function for declaring and defining
 * functions conforming to factor_div_fn().
 */
#define FACTOR_DIV(name) \
double name ## _div (const Factor *f, const Factor *f2)

/* FACTOR_INIT(): macro function for declaring and defining
 * functions conforming to factor_init_fn().
 */
#define FACTOR_INIT(name) \
int name ## _init (Factor *f)

/* FACTOR_RESIZE(): macro function for declaring and defining
 * functions conforming to factor_resize_fn().
 */
#define FACTOR_RESIZE(name) \
int name ## _resize (Factor *f, size_t D, size_t P, size_t K)

/* FACTOR_KERNEL(): macro function for declaring and defining
 * functions conforming to factor_kernel_fn().
 */
#define FACTOR_KERNEL(name) \
char* name ## _kernel (const Factor *f, size_t p0)

/* FACTOR_SET(): macro function for declaring and defining
 * functions conforming to factor_set_fn().
 */
#define FACTOR_SET(name) \
int name ## _set (Factor *f, size_t i, double value)

/* FACTOR_COPY(): macro function for declaring and defining
 * functions conforming to factor_copy_fn().
 */
#define FACTOR_COPY(name) \
int name ## _copy (const Factor *f, Factor *fdup)

/* FACTOR_FREE(): macro function for declaring and defining
 * functions conforming to factor_free_fn().
 */
#define FACTOR_FREE(name) \
void name ## _free (Factor *f)

/* FACTOR_PROP(): macro function for inserting a static factor
 * property into a PyGetSetDef array.
 */
#define FACTOR_PROP(Typ,name) \
  { #name, \
    (getter) Typ ## _get_ ## name, \
    (setter) Typ ## _set_ ## name, \
    Typ ## _getset_ ## name ## _doc, \
    NULL }

/* FACTOR_PROP_GETSET(): macro function for defining property
 * getter and setter functions for static factor properties.
 */
#define FACTOR_PROP_GETSET(Typ,name,idx) \
  FACTOR_PROP_GET(Typ, name, idx) \
  FACTOR_PROP_SET(Typ, name, idx)

/* FACTOR_PROP_GET(): macro function for defining a getter function
 * for static factor properties.
 */
#define FACTOR_PROP_GET(Typ,name,idx) \
static PyObject* Typ ## _get_ ## name (Typ *self) { \
  return PyFloat_FromDouble(factor_get((Factor*) self, idx)); }

/* FACTOR_PROP_SET(): macro function for defining a setter function
 * for static factor properties.
 */
#define FACTOR_PROP_SET(Typ,name,idx) \
static int Typ ## _set_ ## name (Typ *self, PyObject *value, void *cl) { \
  const double v = PyFloat_AsDouble(value); \
  if (PyErr_Occurred()) return -1; \
  if (!factor_set((Factor*) self, idx, v)) { \
    PyErr_SetString(PyExc_ValueError, \
      "failed to set '" #name "' parameter"); \
    return -1; } \
  return 0; }

/* struct factor: structure for holding a variational factor.
 *
 * each factor holds information on a set of @M basis elements,
 * corresponding to @M model weights. factors accept inputs of
 * dimensionality @D, and have a set number of variational
 * parameters, given by @P.
 */
struct factor {
  /* object base. */
  PyObject_HEAD

  /* factor specialization functions:
   *
   *  expectations:
   *   @eval: value at mode.
   *   @mean: first moment.
   *   @var: second moment.
   *   @cov: covariance.
   *
   *  gradients:
   *   @diff_mean: gradient of the first moment.
   *   @diff_var: gradient of the second moment.
   *
   *  mean-field:
   *   @meanfield: update function.
   *
   *  divergence:
   *   @div: kl-divergence between two factors of the same type.
   *
   *  maintenance hooks:
   *   @init: hook for initialization.
   *   @resize: hook for resize handling.
   *   @kernel: hook for kernel construction.
   *   @set: hook for setting parameter values.
   *   @copy: hook for copying extra memory between factors.
   *   @free: hook for extra functionality during deallocation.
   */
  factor_mean_fn      eval;
  factor_mean_fn      mean;
  factor_var_fn       var;
  factor_cov_fn       cov;
  factor_diff_mean_fn diff_mean;
  factor_diff_var_fn  diff_var;
  factor_meanfield_fn meanfield;
  factor_div_fn       div;
  factor_init_fn      init;
  factor_resize_fn    resize;
  factor_kernel_fn    kernel;
  factor_set_fn       set;
  factor_copy_fn      copy;
  factor_free_fn      free;

  /* factor sizes:
   *  @D: number of dimensions.
   *  @P: number of parameters.
   *  @K: number of weights.
   */
  size_t D, P, K;

  /* univariate factors only:
   *  @d: input dimension.
   */
  size_t d;

  /* factor flags:
   *  @fixed: whether to fix the factor to its current state.
   */
  int fixed;

  /* storage of core data:
   *  @inf: fisher information matrix.
   *  @par: parameter vector.
   */
  Matrix *inf;
  Vector *par;
};

/* function declarations (factor-core.c): */

void Factor_reset (Factor *f);

Factor *factor_copy (const Factor *f);

int factor_resize (Factor *f, size_t D, size_t P, size_t K);

size_t factor_dims (const Factor *f);

size_t factor_parms (const Factor *f);

size_t factor_weights (const Factor *f);

double factor_get (const Factor *f, size_t i);

int factor_set (Factor *f, size_t i, double value);

void factor_fix (Factor *f, int fixed);

double factor_eval (const Factor *f, const Vector *x,
                    size_t p, size_t i);

double factor_mean (const Factor *f, const Vector *x,
                    size_t p, size_t i);

double factor_var (const Factor *f, const Vector *x,
                   size_t p, size_t i, size_t j);

double factor_cov (const Factor *f, const Vector *x1, const Vector *x2,
                   size_t p1, size_t p2);

int factor_diff_mean (const Factor *f, const Vector *x,
                      size_t p, size_t i, Vector *df);

int factor_diff_var (const Factor *f, const Vector *x,
                     size_t p, size_t i, size_t j,
                     Vector *df);

int factor_meanfield (Factor *f, const Factor *fp, const Datum *dat,
                      Vector *b, Matrix *B);

double factor_div (const Factor *f, const Factor *f2);

char *factor_kernel (const Factor *f, size_t p0);

#endif /* !__VFL_FACTOR_H__ */

