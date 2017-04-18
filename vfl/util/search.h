
/* ensure once-only inclusion. */
#ifndef __VFL_SEARCH_H__
#define __VFL_SEARCH_H__

/* include the opencl header. */
#include <CL/opencl.h>

/* include required vfl headers. */
#include <vfl/model.h>
#include <vfl/data.h>

/* search_t: structure for holding the state of a variance search.
 */
typedef struct {
  /* associated core structures:
   *  @mdl: model used to build kernel code strings.
   *  @dat: dataset used for individual searches.
   */
  model_t *mdl;
  data_t *dat;

  /* opencl core variables:
   *  @dev: compute device identifier.
   *  @ctx: compute context.
   *  @queue: command queue.
   *  @prog: compiled program.
   *  @kern: compute kernel.
   *  @src: program code string.
   */
  cl_device_id     dev;
  cl_context       ctx;
  cl_command_queue queue;
  cl_program       prog;
  cl_kernel        kern;
  char *src;

  /* host-side calculation variables:
   *  @xgrid: grid input location matrix.
   *  @xdat: data input location matrix.
   *  @xmax: location of current maximum.
   *  @pdat: data output index vector.
   *  @par: kernel parameter vector.
   *  @C: inverse covariance matrix.
   *  @vmax: current maximum variance.
   */
  float *xgrid, *xdat, *xmax, *par, *C;
  unsigned int *pdat;
  float vmax;

  /* opencl device memory addresses:
   *
   *  inputs:
   *   @dev_xgrid: grid input location matrix.
   *   @dev_xdat: data input location matrix.
   *   @dev_pdat: data output index vector.
   *   @dev_par: kernel parameter vector.
   *   @dev_C: inverse covariance matrix.
   *
   *  outputs:
   *   @dev_var: computed variance result.
   */
  cl_mem dev_xgrid;
  cl_mem dev_xdat;
  cl_mem dev_pdat;
  cl_mem dev_par;
  cl_mem dev_C;
  cl_mem dev_var;
}
search_t;

/* function declarations (search.c): */

search_t *search_alloc (model_t *mdl, data_t *dat,
                        const matrix_t *grid);

void search_free (search_t *S);

int search_execute (search_t *S, vector_t *x);

#endif /* !__VFL_SEARCH_H__ */

