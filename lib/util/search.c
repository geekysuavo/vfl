
/* include the search header. */
#include <vfl/util/search.h>

/* * * * documentation of opencl kernel code: * * * */

/* vfl_covkernel(): evaluate the covariance function of
 * a gaussian process between any two outputs of a model
 * function.
 *
 * arguments:
 *  @par: array of kernel parameters.
 *  @x1: first D-dimensional input location.
 *  @x2: second D-dimensional input location.
 *  @p1: first function output index.
 *  @p2: second function output index.
 *  @D: dimensionality of the input space.
 *
 * returns:
 *  covariance between function outputs (x1,p1) and (x2,p2).
 */

/* vfl_variance(): opencl kernel for computing the posterior predictive
 * variance of a given gaussian process. the variances of all outputs
 * are summed together.
 *
 * in order to conserve device memory, this kernel performs O(N^2)
 * calls to the covariance function during the summation over the
 * elements of the inverse covariance matrix.
 *
 * arguments:
 *  @xgrid: array of D*N floats holding the grid input locations.
 *  @xdat: array of D*n floats holding the data input locations.
 *  @pdat: array of n uints holding the data output indices.
 *  @par: array of kernel parameters, size unspecified.
 *  @C: array of n*n floats holding the inverse covariance matrix.
 *  @D: number of input dimensions.
 *  @K: number of function outputs.
 *  @N: number of grid values in the computation.
 *  @n: number of observations in the dataset.
 *  @var: array of N floats holding the output variance calculations.
 */

/* SEARCH_FORMAT: constant format string used to generate opencl
 * program source code for searching the posterior variance of
 * gaussian processes derived from variational feature
 * regression models.
 */
#define SEARCH_FORMAT "\n" \
"inline float vfl_covkernel (const __global float *par,"            "\n" \
"                            const __global float *x1,"             "\n" \
"                            const __global float *x2,"             "\n" \
"                            const unsigned int p1,"                "\n" \
"                            const unsigned int p2,"                "\n" \
"                            const unsigned int D) {"               "\n" \
"  /* initialize the covariance computation. */"                    "\n" \
"  float cov, sum = 0.0f, diag = 1.0f;"                             "\n" \
""                                                                  "\n" \
"  /* get the global kernel parameters. */"                         "\n" \
"  const float tau = par[0];"                                       "\n" \
"  const float nu = par[1];"                                        "\n" \
""                                                                  "\n" \
"  /* begin model-generated kernel code. */"                        "\n" \
"  %s"                                                              "\n" \
"  /* end model-generated kernel code. */"                          "\n" \
""                                                                  "\n" \
"  /* compute the diagonal noise term. */"                          "\n" \
"  for (unsigned int d = 0; d < D; d++)"                            "\n" \
"    diag *= (x1[d] == x2[d]);"                                     "\n" \
""                                                                  "\n" \
"  /* return the computed result. */"                               "\n" \
"  return (sum / nu + diag) / tau;"                                 "\n" \
"}"                                                                 "\n" \
""                                                                  "\n" \
"__kernel void vfl_variance (const __global float *xgrid,"          "\n" \
"                            const __global float *xdat,"           "\n" \
"                            const __global uint  *pdat,"           "\n" \
"                            const __global float *par,"            "\n" \
"                            const __global float *C,"              "\n" \
"                            const unsigned int D,"                 "\n" \
"                            const unsigned int K,"                 "\n" \
"                            const unsigned int N,"                 "\n" \
"                            const unsigned int n,"                 "\n" \
"                            __global float *var) {"                "\n" \
"  /* get the grid index. */"                                       "\n" \
"  const unsigned int gid = get_global_id(0);"                      "\n" \
""                                                                  "\n" \
"  /* avoid buffer overflow. */"                                    "\n" \
"  if (gid >= N)"                                                   "\n" \
"    return;"                                                       "\n" \
""                                                                  "\n" \
"  /* get the current grid location. */"                            "\n" \
"  const __global float *xs = xgrid + (gid * D);"                   "\n" \
""                                                                  "\n" \
"  /* initialize the variance computation. */"                      "\n" \
"  float sum = 0.0f;"                                               "\n" \
""                                                                  "\n" \
"  /* sum variances of each output together. */"                    "\n" \
"  for (unsigned int ps = 0; ps < K; ps++) {"                       "\n" \
"    /* include the auto-covariance contribution. */"               "\n" \
"    sum += vfl_covkernel(par, xs, xs, ps, ps, D);"                 "\n" \
""                                                                  "\n" \
"    /* loop over the first matrix dimension. */"                   "\n" \
"    for (unsigned int i = 0, cidx = 0; i < n; i++) {"              "\n" \
"      /* get the first data value. */"                             "\n" \
"      const __global float *xi = xdat + (i * D);"                  "\n" \
"      unsigned int pi = pdat[i];"                                  "\n" \
""                                                                  "\n" \
"      /* loop over the second matrix dimension. */"                "\n" \
"      for (unsigned int j = 0; j < n; j++, cidx++) {"              "\n" \
"        /* get the second data value. */"                          "\n" \
"        const __global float *xj = xdat + (j * D);"                "\n" \
"        unsigned int pj = pdat[j];"                                "\n" \
""                                                                  "\n" \
"        /* include the current matrix element contribution. */"    "\n" \
"        sum -= vfl_covkernel(par, xs, xi, ps, pi, D)"              "\n" \
"             * vfl_covkernel(par, xj, xs, pj, ps, D)"              "\n" \
"             * C[cidx];"                                           "\n" \
"      }"                                                           "\n" \
"    }"                                                             "\n" \
"  }"                                                               "\n" \
""                                                                  "\n" \
"  /* store the computed result. */"                                "\n" \
"  var[gid] = sum;"                                                 "\n" \
"}\n"

/* * * * static function definitions: * * * */

/* free_buffers(): free all allocated calculation-related buffers
 * within a search structure.
 *
 * arguments:
 *  @S: search structure pointer.
 */
static void free_buffers (search_t *S) {
  /* free the host-side calculation variables. */
  free(S->par);

  /* free the device-side memory objects. */
  clReleaseMemObject(S->dev_par);
  clReleaseMemObject(S->dev_var);
  clReleaseMemObject(S->dev_xgrid);
  clReleaseMemObject(S->dev_xdat);
  clReleaseMemObject(S->dev_pdat);
  clReleaseMemObject(S->dev_C);

  /* reset the host-side pointers. */
  S->par = S->var = S->xgrid = S->xmax = S->xdat = S->C = NULL;
  S->pdat = NULL;

  /* reset the device-side pointers. */
  S->dev_par = S->dev_var = S->dev_xgrid = NULL;
  S->dev_xdat = S->dev_pdat = S->dev_C = NULL;
}

/* refresh_buffers(): ensure that all allocated calculation-related
 * buffers have the correct size and are initialized properly.
 *
 * arguments:
 *  @S: search structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
static int refresh_buffers (search_t *S) {
  /* get the new sizes. */
  const unsigned int D = S->mdl->D;
  const unsigned int P = S->mdl->P;
  const unsigned int n = S->dat->N;
  const unsigned int N = data_count_grid(S->grid, NULL);

  /* check for any differences. */
  if (S->D != D || S->P != P || S->N != N || S->n != n) {
    /* free the buffers. */
    free_buffers(S);

    /* determine the sizes of the buffers. */
    const size_t num_par   = sizeof(cl_float) * P;
    const size_t num_var   = sizeof(cl_float) * N;
    const size_t num_xgrid = sizeof(cl_float) * N;
    const size_t num_xmax  = sizeof(cl_float) * D;
    const size_t num_xdat  = sizeof(cl_float) * D * n;
    const size_t num_C     = sizeof(cl_float) * n * n;
    const size_t num_pdat  = sizeof(cl_uint)  * n;

    /* determine the amount of host floats to allocate. */
    const size_t bytes = num_par + num_var + num_xgrid + num_xmax
                       + num_xdat + num_pdat + num_C;

    /* allocate the host-side memory block. */
    char *ptr = malloc(bytes);
    if (!ptr)
      return 0;

    /* initialize par. */
    S->par = (cl_float*) ptr;
    ptr += num_par;

    /* initialize var. */
    S->var = (cl_float*) ptr;
    ptr += num_var;

    /* initialize xgrid. */
    S->xgrid = (cl_float*) ptr;
    ptr += num_xgrid;

    /* initialize xmax. */
    S->xmax = (cl_float*) ptr;
    ptr += num_xmax;

    /* initialize xdat. */
    S->xdat = (cl_float*) ptr;
    ptr += num_xdat;

    /* initialize C. */
    S->C = (cl_float*) ptr;
    ptr += num_C;

    /* initialize pdat. */
    S->pdat = (cl_uint*) ptr;

    /* allocate the device-side buffer: par. */
    S->dev_par = clCreateBuffer(S->ctx, CL_MEM_READ_ONLY,
                                num_par, NULL, NULL);

    /* allocate the device-side buffer: var. */
    S->dev_var = clCreateBuffer(S->ctx, CL_MEM_WRITE_ONLY,
                                num_var, NULL, NULL);

    /* allocate the device-side buffer: xgrid. */
    S->dev_xgrid = clCreateBuffer(S->ctx, CL_MEM_READ_ONLY,
                                  num_xgrid, NULL, NULL);

    /* allocate the device-side buffer: xdat. */
    S->dev_xdat = clCreateBuffer(S->ctx, CL_MEM_READ_ONLY,
                                 num_xdat, NULL, NULL);

    /* allocate the device-side buffer: pdat. */
    S->dev_pdat = clCreateBuffer(S->ctx, CL_MEM_READ_ONLY,
                                 num_pdat, NULL, NULL);

    /* allocate the device-side buffer: C. */
    S->dev_C = clCreateBuffer(S->ctx, CL_MEM_READ_ONLY,
                              num_C, NULL, NULL);

    /* check for allocation failures. */
    if (!S->dev_par || !S->dev_var || !S->dev_xgrid ||
        !S->dev_xdat || !S->dev_pdat || !S->dev_C)
      return 0;

    /* store the new sizes. */
    S->D = D;
    S->P = P;
    S->N = N;
    S->n = n;
  }

  /* return success. */
  return 1;
}

/* * * * function definitions: * * * */

/* search_alloc(): allocate a new structure for variance searches.
 *
 * arguments:
 *  @mdl: model to use for constructing the search.
 *  @dat: dataset to pull observations from.
 *  @grid: matrix of gridding information.
 *
 * returns:
 *  newly allocated and initialized search structure.
 */
search_t *search_alloc (model_t *mdl, data_t *dat,
                        const matrix_t *grid) {
  /* check the input structure pointers. */
  if (!mdl || !dat | !grid)
    return NULL;

  /* allocate a new structure pointer. */
  search_t *S = malloc(sizeof(search_t));
  if (!S)
    return NULL;

  /* store the model and dataset. */
  S->grid = grid;
  S->mdl = mdl;
  S->dat = dat;

  /* initialize the buffer sizes. */
  S->D = S->P = S->N = S->n = 0;

  /* initialize the opencl variables. */
  S->plat = NULL;
  S->dev = NULL;
  S->ctx = NULL;
  S->queue = NULL;
  S->prog = NULL;
  S->kern = NULL;
  S->src = NULL;

  /* initialize the host-side pointers. */
  S->xgrid = S->xdat = S->xmax = S->par = S->C = S->var = NULL;
  S->pdat = NULL;
  S->vmax = 0.0;

  /* initialize the device-side pointers. */
  S->dev_xgrid = S->dev_xdat = S->dev_pdat = NULL;
  S->dev_par = S->dev_C = S->dev_var = NULL;

  /* generate the model kernel code. */
  char *ksrc = model_kernel(S->mdl);
  if (!ksrc)
    goto fail;

  /* allocate memory for the complete program code. */
  const unsigned int len = strlen(SEARCH_FORMAT) + strlen(ksrc) + 8;
  S->src = malloc(len);
  if (!S->src) {
    free(ksrc);
    goto fail;
  }

  /* write the complete program code and free the model kernel code. */
  sprintf(S->src, SEARCH_FORMAT, ksrc);
  free(ksrc);

  /* get the first available compute platform. */
  int ret = clGetPlatformIDs(1, &S->plat, NULL);
  if (ret != CL_SUCCESS)
    goto fail;

  /* connect to the first available compute device. */
  ret = clGetDeviceIDs(S->plat, CL_DEVICE_TYPE_GPU, 1, &S->dev, NULL);
  if (ret != CL_SUCCESS)
    goto fail;

  /* create a compute context. */
  S->ctx = clCreateContext(NULL, 1, &S->dev, NULL, NULL, &ret);
  if (!S->ctx)
    goto fail;

  /* create a command queue. */
  S->queue = clCreateCommandQueue(S->ctx, S->dev, 0, &ret);
  if (!S->queue)
    goto fail;

  /* create the compute program. */
  S->prog = clCreateProgramWithSource(S->ctx, 1, (const char**) &S->src,
                                      NULL, &ret);

  /* check for program creation failure. */
  if (!S->prog)
    goto fail;

  /* build the program executable. */
  ret = clBuildProgram(S->prog, 0, NULL, NULL, NULL, NULL);
  if (ret != CL_SUCCESS)
    goto fail;

  /* create the compute kernel. */
  S->kern = clCreateKernel(S->prog, "vfl_variance", &ret);
  if (!S->kern || ret != CL_SUCCESS)
    goto fail;

  /* return the new structure pointer. */
  return S;

fail:
  /* free the search structure pointer and return null. */
  search_free(S);
  return NULL;
}

/* search_free(): free a variance search structure.
 *
 * arguments:
 *  @S: pointer to a search structure to free.
 */
void search_free (search_t *S) {
  /* return if the structure pointer is null. */
  if (!S)
    return;

  /* release the opencl variables. */
  clReleaseProgram(S->prog);
  clReleaseKernel(S->kern);
  clReleaseCommandQueue(S->queue);
  clReleaseContext(S->ctx);

  /* free the kernel source code string. */
  free(S->src);

  /* free the calculation buffers. */
  free_buffers(S);

  /* free the structure pointer. */
  free(S);
}

/* search_execute(): perform a search procedure to locate the
 * maximum of the posterior predictive variance of a gaussian
 * process.
 *
 * arguments:
 *  @S: search structure pointer to access for searching.
 *  @x: vector structure pointer to store the output into.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int search_execute (search_t *S, vector_t *x) {
  /* check the input structure pointers. */
  if (!S || !x)
    return 0;

  /* refresh the calculation buffers. */
  if (!refresh_buffers(S))
    return 0;

  /* FIXME: implement search_execute():
   *
   *  1. fill S.par from S.mdl
   *  2. fill S.xgrid from S.grid
   *  3. fill S.xdat, S.pdat from S.dat
   *  4. compute S.C using S.mdl, S.dat
   *  5. enqueue host->dev buffer xfers
   *  6. set up kernel arguments.
   *  7. enqueue the kernel call.
   *  8. enqueue dev->host buffer xfer
   *  9. loop to find the maximum, store in x
   */

  /* return success. */
  return 1;
}

