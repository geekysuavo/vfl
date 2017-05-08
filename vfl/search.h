
/* ensure once-only inclusion. */
#ifndef __VFL_SEARCH_H__
#define __VFL_SEARCH_H__

/* include vfl headers. */
#include <vfl/base/object.h>
#include <vfl/model.h>

/* if required, include the opencl header. */
#ifdef __VFL_USE_OPENCL
# ifdef __APPLE__
#  include <OpenCL/opencl.h>
# else
#  include <CL/opencl.h>
# endif
#endif

/* OBJECT_IS_SEARCH(): check if an object is a search structure.
 */
#define OBJECT_IS_SEARCH(obj) \
  (OBJECT_TYPE(obj) == vfl_object_search)

/* search_t: structure for holding the state of a variance search.
 */
typedef struct {
  /* base structure members. */
  OBJECT_BASE;

  /* associated core structures:
   *  @grid: matrix of gridding information.
   *  @mdl: model used to build kernel code strings.
   *  @dat: dataset used for individual searches.
   */
  matrix_t *grid;
  model_t *mdl;
  data_t *dat;

  /* current buffer states:
   *  @D: dimension count.
   *  @P: parameter count.
   *  @K: output count.
   *  @G: grid total size.
   *  @N: grid value count.
   *  @n: observation count.
   */
#ifdef __VFL_USE_OPENCL
  cl_uint D, P, K, G, N, n;
#else
  unsigned int D, P, K, G, N, n;
#endif

  /* memory utilization and execution control variables:
   *  @sz_*: in-memory sizes (host and device).
   *  @wgsize: work-group size.
   */
  size_t sz_par, sz_var, sz_xgrid, sz_xmax, sz_xdat, sz_cblk, sz_C, sz_pdat;
  size_t wgsize;

  /* opencl core variables:
   *  @plat: compute platform identifier.
   *  @dev: compute device identifier.
   *  @ctx: compute context.
   *  @queue: command queue.
   *  @prog: compiled program.
   *  @kern: compute kernel.
   *  @src: program code string.
   */
#ifdef __VFL_USE_OPENCL
  cl_platform_id   plat;
  cl_device_id     dev;
  cl_context       ctx;
  cl_command_queue queue;
  cl_program       prog;
  cl_kernel        kern;
  char *src;
#endif

  /* host-side calculation variables:
   *  @par: kernel parameter vector.
   *  @var: output variance result.
   *  @xgrid: grid input location matrix.
   *  @xmax: location of current maximum.
   *  @xdat: data input location matrix.
   *  @pdat: data output index vector.
   *  @C: inverse covariance matrix.
   *  @cov: double-precision covariance matrix.
   *  @vmax: current maximum variance.
   */
#ifdef __VFL_USE_OPENCL
  cl_double *par, *var, *xgrid, *xmax, *xdat, *C;
  cl_uint *pdat;
#else
  vector_t *cs;
#endif
  matrix_t *cov;
  float vmax;

  /* opencl device memory addresses:
   *
   *  inputs:
   *   @dev_par: kernel parameter vector.
   *   @dev_var: output variance vector.
   *   @dev_xgrid: grid input location matrix.
   *   @dev_xdat: data input location matrix.
   *   @dev_pdat: data output index vector.
   *   @dev_cblk: kernel vector elements.
   *   @dev_C: inverse covariance matrix.
   *
   *  outputs:
   *   @dev_var: computed variance result.
   */
#ifdef __VFL_USE_OPENCL
  cl_mem dev_par;
  cl_mem dev_var;
  cl_mem dev_xgrid;
  cl_mem dev_xdat;
  cl_mem dev_pdat;
  cl_mem dev_cblk;
  cl_mem dev_C;
#endif
}
search_t;

/* function declarations (search.c): */

#define search_alloc() \
  (search_t*) obj_alloc(vfl_object_search);

int search_set_model (search_t *S, model_t *mdl);

int search_set_data (search_t *S, data_t *dat);

int search_set_grid (search_t *S, matrix_t *grid);

int search_set_outputs (search_t *S, const unsigned int num);

int search_execute (search_t *S, vector_t *x);

/* available object types: */

extern const object_type_t *vfl_object_search;

#endif /* !__VFL_SEARCH_H__ */
