
/* ensure once-only inclusion. */
#ifndef __VFL_SEARCH_H__
#define __VFL_SEARCH_H__

/* include required vfl headers. */
#include <vfl/data.h>
#include <vfl/model.h>
#include <vfl/util/blas.h>

/* include the opencl header. */
#include <CL/opencl.h>

/* search_t: structure for holding the state of a variance search.
 */
typedef struct {
  /* associated core structures:
   *  @grid: matrix of gridding information.
   *  @mdl: model used to build kernel code strings.
   *  @dat: dataset used for individual searches.
   */
  const matrix_t *grid;
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
  cl_uint D, P, K, G, N, n;

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
  cl_platform_id   plat;
  cl_device_id     dev;
  cl_context       ctx;
  cl_command_queue queue;
  cl_program       prog;
  cl_kernel        kern;
  char *src;

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
  cl_double *par, *var, *xgrid, *xmax, *xdat, *C;
  cl_uint *pdat;
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
  cl_mem dev_par;
  cl_mem dev_var;
  cl_mem dev_xgrid;
  cl_mem dev_xdat;
  cl_mem dev_pdat;
  cl_mem dev_cblk;
  cl_mem dev_C;
}
search_t;

/* function declarations (search.c): */

search_t *search_alloc (model_t *mdl, data_t *dat,
                        const matrix_t *grid);

void search_free (search_t *S);

int search_set_outputs (search_t *S, const unsigned int num);

int search_execute (search_t *S, vector_t *x);

#endif /* !__VFL_SEARCH_H__ */

