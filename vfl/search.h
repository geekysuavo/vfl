
/* ensure once-only inclusion. */
#ifndef __VFL_SEARCH_H__
#define __VFL_SEARCH_H__

/* include vfl headers. */
#include <vfl/model.h>

/* if required, include the opencl header. */
#ifdef __VFL_USE_OPENCL
# ifdef __APPLE__
#  include <OpenCL/opencl.h>
# else
#  include <CL/opencl.h>
# endif
#endif

/* Search_Check(): macro to check if a PyObject is a Search.
 */
#define Search_Check(v) (Py_TYPE(v) == &Search_Type)

/* Search_Type: globally available search type structure.
 */
PyAPI_DATA(PyTypeObject) Search_Type;

/* Search: structure for holding the state of a variance search.
 */
typedef struct {
  /* object base. */
  PyObject_HEAD

  /* associated core structures:
   *  @grid: matrix of gridding information.
   *  @mdl: model used to build kernel code strings.
   *  @dat: dataset used for individual searches.
   */
  Matrix *grid;
  Model *mdl;
  Data *dat;

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
  size_t D, P, K, G, N, n;
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
  Vector *cs;
#endif
  Matrix *cov;
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
Search;

/* function declarations (search-core.c): */

int search_set_model (Search *S, Model *mdl);

int search_set_data (Search *S, Data *dat);

int search_set_grid (Search *S, Matrix *grid);

int search_set_outputs (Search *S, size_t num);

int search_execute (Search *S, Vector *x);

#endif /* !__VFL_SEARCH_H__ */

