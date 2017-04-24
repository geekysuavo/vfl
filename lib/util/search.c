
/* include the search and gridding headers. */
#include <vfl/util/search.h>
#include <vfl/util/grid.h>

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
 *  @C: array of n*(n+1)/2 floats holding the inverse covariance matrix.
 *  @D: number of input dimensions.
 *  @K: number of function outputs.
 *  @N: number of grid values in the computation.
 *  @n: number of observations in the dataset.
 *  @var: array of N floats holding the output variance calculations.
 */

/* SEARCH_MAX_GRID: maximum number of grid points searched at once.
 */
#define SEARCH_MAX_GRID 256

/* SEARCH_FORMAT: constant format string used to generate opencl
 * program source code for searching the posterior variance of
 * gaussian processes derived from variational feature
 * regression models.
 */
#define SEARCH_FORMAT "\n" \
"#pragma OPENCL EXTENSION cl_khr_fp64 : enable"                     "\n" \
""                                                                  "\n" \
"inline double vfl_covkernel (const __global double *par,"          "\n" \
"                             const __global double *x1,"           "\n" \
"                             const __global double *x2,"           "\n" \
"                             const uint p1,"                       "\n" \
"                             const uint p2,"                       "\n" \
"                             const uint D) {"                      "\n" \
"  /* initialize the covariance computation. */"                    "\n" \
"  double cov, sum = 0.0;"                                          "\n" \
""                                                                  "\n" \
"  /* get the global kernel parameters. */"                         "\n" \
"  const double scale = par[0];"                                    "\n" \
""                                                                  "\n" \
"  /* begin model-generated kernel code. */"                        "\n" \
"  %s"                                                              "\n" \
"  /* end model-generated kernel code. */"                          "\n" \
""                                                                  "\n" \
"  /* return the computed result. */"                               "\n" \
"  return scale * sum;"                                             "\n" \
"}"                                                                 "\n" \
""                                                                  "\n" \
"__kernel void vfl_variance (const __global double *xgrid,"         "\n" \
"                            const __global double *xdat,"          "\n" \
"                            const __global uint   *pdat,"          "\n" \
"                            const __global double *par,"           "\n" \
"                            const __global double *C,"             "\n" \
"                            const uint D, const uint K,"           "\n" \
"                            const uint N, const uint n,"           "\n" \
"                            __global double *var) {"               "\n" \
"  /* get the grid index. */"                                       "\n" \
"  const size_t gid = get_global_id(0);"                            "\n" \
""                                                                  "\n" \
"  /* avoid buffer overflow. */"                                    "\n" \
"  if (gid >= N)"                                                   "\n" \
"    return;"                                                       "\n" \
""                                                                  "\n" \
"  /* get the current grid location. */"                            "\n" \
"  const __global double *xs = xgrid + (gid * D);"                  "\n" \
""                                                                  "\n" \
"  /* initialize the variance computation. */"                      "\n" \
"  double sum = 0.0;"                                               "\n" \
""                                                                  "\n" \
"  /* sum variances of each output together. */"                    "\n" \
"  for (uint ps = 0; ps < K; ps++) {"                               "\n" \
"    /* include the auto-covariance contribution. */"               "\n" \
"    sum += vfl_covkernel(par, xs, xs, ps, ps, D);"                 "\n" \
""                                                                  "\n" \
"    /* loop over each matrix row. */"                              "\n" \
"    for (uint i = 0, cidx = 0; i < n; i++) {"                      "\n" \
"      /* get the first data value. */"                             "\n" \
"      const __global double *xi = xdat + (i * D);"                 "\n" \
"      uint pi = pdat[i];"                                          "\n" \
""                                                                  "\n" \
"      /* compute the 'left-hand' term. */"                         "\n" \
"      const double sii = vfl_covkernel(par, xs, xi, ps, pi, D);"   "\n" \
""                                                                  "\n" \
"      /* loop over the off-diagonal row elements. */"              "\n" \
"      for (uint j = 0; j < i; j++, cidx++) {"                      "\n" \
"        /* get the second data value. */"                          "\n" \
"        const __global double *xj = xdat + (j * D);"               "\n" \
"        uint pj = pdat[j];"                                        "\n" \
""                                                                  "\n" \
"        /* include the off-diagonal matrix elements. */"           "\n" \
"        sum -= 2.0 * vfl_covkernel(par, xj, xs, pj, ps, D)"        "\n" \
"                   * sii * C[cidx];"                               "\n" \
"      }"                                                           "\n" \
""                                                                  "\n" \
"      /* include the diagonal matrix element. */"                  "\n" \
"      sum -= sii * sii * C[cidx++];"                               "\n" \
"    }"                                                             "\n" \
"  }"                                                               "\n" \
""                                                                  "\n" \
"  /* store the computed result. */"                                "\n" \
"  var[gid] = sum;"                                                 "\n" \
"}\n"

/* * * * static function definitions: * * * */

/* set_arguments(): set the kernel arguments of a search structure.
 *
 * arguments:
 *  @S: search structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
static int set_arguments (search_t *S) {
  /* initialize the status code. */
  int ret = CL_SUCCESS;

  /* set all arguments. */
  ret |= clSetKernelArg(S->kern, 0, sizeof(cl_mem),  &S->dev_xgrid);
  ret |= clSetKernelArg(S->kern, 1, sizeof(cl_mem),  &S->dev_xdat);
  ret |= clSetKernelArg(S->kern, 2, sizeof(cl_mem),  &S->dev_pdat);
  ret |= clSetKernelArg(S->kern, 3, sizeof(cl_mem),  &S->dev_par);
  ret |= clSetKernelArg(S->kern, 4, sizeof(cl_mem),  &S->dev_C);
  ret |= clSetKernelArg(S->kern, 5, sizeof(cl_uint), &S->D);
  ret |= clSetKernelArg(S->kern, 6, sizeof(cl_uint), &S->K);
  ret |= clSetKernelArg(S->kern, 7, sizeof(cl_uint), &S->N);
  ret |= clSetKernelArg(S->kern, 8, sizeof(cl_uint), &S->n);
  ret |= clSetKernelArg(S->kern, 9, sizeof(cl_mem),  &S->dev_var);

  /* return the resulting status code. */
  return (ret == CL_SUCCESS);
}

/* free_buffers(): free all allocated calculation-related buffers
 * within a search structure.
 *
 * arguments:
 *  @S: search structure pointer.
 */
static void free_buffers (search_t *S) {
  /* free the dense covariance matrix. */
  matrix_free(S->cov);
  S->cov = NULL;

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
  const unsigned int P = S->mdl->P + 1;
  const unsigned int D = S->mdl->D;
  const unsigned int n = S->dat->N;

  /* determine the total grid size. */
  unsigned int G;
  grid_iterator_alloc(S->grid, &G, NULL, NULL, NULL);

  /* check if the grid size exceeds the maximum grid size. */
  unsigned int N = G;
  if (N > SEARCH_MAX_GRID)
    N = SEARCH_MAX_GRID;

  /* check for any differences. */
  if (S->D != D || S->P != P || S->N != N || S->n != n) {
    /* free the buffers. */
    free_buffers(S);

    /* determine the sizes of the buffers. */
    S->sz_par   = sizeof(cl_double) * P;
    S->sz_var   = sizeof(cl_double) * N;
    S->sz_xgrid = sizeof(cl_double) * N;
    S->sz_xmax  = sizeof(cl_double) * D;
    S->sz_xdat  = sizeof(cl_double) * D * n;
    S->sz_C     = sizeof(cl_double) * (n * (n + 1)) / 2;
    S->sz_pdat  = sizeof(cl_uint)   * n;

    /* determine the amount of host floats to allocate. */
    const size_t bytes = S->sz_par + S->sz_var + S->sz_xgrid
                       + S->sz_xmax + S->sz_xdat + S->sz_pdat
                       + S->sz_C;

    /* allocate the dense covariance matrix. */
    S->cov = matrix_alloc(n, n);
    if (!S->cov)
      return 0;

    /* allocate the host-side memory block. */
    char *ptr = malloc(bytes);
    if (!ptr)
      return 0;

    /* initialize par. */
    S->par = (cl_double*) ptr;
    ptr += S->sz_par;

    /* initialize var. */
    S->var = (cl_double*) ptr;
    ptr += S->sz_var;

    /* initialize xgrid. */
    S->xgrid = (cl_double*) ptr;
    ptr += S->sz_xgrid;

    /* initialize xmax. */
    S->xmax = (cl_double*) ptr;
    ptr += S->sz_xmax;

    /* initialize xdat. */
    S->xdat = (cl_double*) ptr;
    ptr += S->sz_xdat;

    /* initialize C. */
    S->C = (cl_double*) ptr;
    ptr += S->sz_C;

    /* initialize pdat. */
    S->pdat = (cl_uint*) ptr;

    /* allocate the device-side buffer: par. */
    S->dev_par = clCreateBuffer(S->ctx, CL_MEM_READ_ONLY,
                                S->sz_par, NULL, NULL);

    /* allocate the device-side buffer: var. */
    S->dev_var = clCreateBuffer(S->ctx, CL_MEM_WRITE_ONLY,
                                S->sz_var, NULL, NULL);

    /* allocate the device-side buffer: xgrid. */
    S->dev_xgrid = clCreateBuffer(S->ctx, CL_MEM_READ_ONLY,
                                  S->sz_xgrid, NULL, NULL);

    /* allocate the device-side buffer: xdat. */
    S->dev_xdat = clCreateBuffer(S->ctx, CL_MEM_READ_ONLY,
                                 S->sz_xdat, NULL, NULL);

    /* allocate the device-side buffer: pdat. */
    S->dev_pdat = clCreateBuffer(S->ctx, CL_MEM_READ_ONLY,
                                 S->sz_pdat, NULL, NULL);

    /* allocate the device-side buffer: C. */
    S->dev_C = clCreateBuffer(S->ctx, CL_MEM_READ_ONLY,
                              S->sz_C, NULL, NULL);

    /* check for allocation failures. */
    if (!S->dev_par || !S->dev_var || !S->dev_xgrid ||
        !S->dev_xdat || !S->dev_pdat || !S->dev_C)
      return 0;

    /* store the new sizes. */
    S->D = D;
    S->P = P;
    S->G = G;
    S->N = N;
    S->n = n;
  }

  /* return success. */
  return 1;
}

/* fill_buffers(): compute the contents of all host-side calculation
 * buffers (except for the grid values).
 *
 * arguments:
 *  @S: search structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
static int fill_buffers (search_t *S) {
  /* store the current noise precision and weight ratio. */
  S->par[0] = 1.0 / (S->mdl->nu * S->mdl->tau);

  /* store the current factor parameters. */
  for (unsigned int j = 0, p0 = 1; j < S->mdl->M; j++) {
    /* get the current factor parameter vector. */
    const vector_t *par = S->mdl->factors[j]->par;

    /* copy the vector elements into the local array. */
    for (unsigned int p = 0; p < par->len; p++)
      S->par[p0 + p] = vector_get(par, p);

    /* increment the array offset. */
    p0 += par->len;
  }

  /* compute the covariance matrix elements. */
  for (unsigned int i = 0; i < S->n; i++) {
    /* get the row-wise observation. */
    datum_t *di = data_get(S->dat, i);

    /* while we're here, store the data array values. */
    S->pdat[i] = di->p;
    for (unsigned int d = 0; d < S->D; d++)
      S->xdat[i * S->D + d] = vector_get(di->x, d);

    /* loop over the elements of each row. */
    for (unsigned int j = 0; j <= i; j++) {
      /* get the column-wise observation. */
      datum_t *dj = data_get(S->dat, j);

      /* compute the covariance matrix element. */
      const double cij = model_cov(S->mdl, di->x, dj->x, di->p, dj->p);

      /* store the computed matrix element. */
      matrix_set(S->cov, i, j, cij);
      if (i != j)
        matrix_set(S->cov, j, i, cij);
    }
  }

  /* compute the cholesky decomposition of the covariance matrix. */
  if (!chol_decomp(S->cov) || !chol_invert(S->cov, S->cov))
    return 0;

  /* pack the inverted matrix into the host-side array. */
  for (unsigned int i = 0, cidx = 0; i < S->n; i++)
    for (unsigned int j = 0; j <= i; j++, cidx++)
      S->C[cidx] = matrix_get(S->cov, i, j);

  /* return success. */
  return 1;
}

/* write_buffers(): write the contents of all host-side calculation
 * buffers to the compute device associated with a search structure.
 *
 * arguments:
 *  @S: search structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
static int write_buffers (search_t *S) {
  /* write xdat to the device. */
  int ret = clEnqueueWriteBuffer(S->queue, S->dev_xdat, CL_FALSE, 0,
                                 S->sz_xdat, S->xdat, 0, NULL, NULL);

  /* write pdat to the device. */
  ret |= clEnqueueWriteBuffer(S->queue, S->dev_pdat, CL_FALSE, 0,
                              S->sz_pdat, S->pdat, 0, NULL, NULL);

  /* write par to the device. */
  ret |= clEnqueueWriteBuffer(S->queue, S->dev_par, CL_FALSE, 0,
                              S->sz_par, S->par, 0, NULL, NULL);

  /* write C to the device. */
  ret |= clEnqueueWriteBuffer(S->queue, S->dev_C, CL_FALSE, 0,
                              S->sz_C, S->C, 0, NULL, NULL);

  /* block until all writes have completed. */
  clFinish(S->queue);

  /* return the resulting status code. */
  return (ret == CL_SUCCESS);
}

/* read_buffers(): read the contents of the device-side result
 * array to host-side memory for further processing.
 *
 * arguments:
 *  @S: search structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
static int read_buffers (search_t *S) {
  /* read the results from the device. */
  int ret = clEnqueueReadBuffer(S->queue, S->dev_var, CL_TRUE, 0,
                                S->sz_var, S->var, 0, NULL, NULL);

  /* return the resulting status code. */
  return (ret == CL_SUCCESS);
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
  if (!mdl || !dat | !grid_validate(grid))
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
  S->D = S->P = S->K = S->N = S->n = 0;

  /* initialize the opencl variables. */
  S->plat = NULL;
  S->dev = NULL;
  S->ctx = NULL;
  S->queue = NULL;
  S->prog = NULL;
  S->kern = NULL;
  S->src = NULL;

  /* initialize the host-side pointers. */
  S->par = S->var = S->xgrid = S->xmax = S->xdat = S->C = NULL;
  S->pdat = NULL;
  S->cov = NULL;
  S->vmax = 0.0;

  /* initialize the device-side pointers. */
  S->dev_par = S->dev_var = S->dev_xgrid = NULL;
  S->dev_xdat = S->dev_pdat = S->dev_C = NULL;

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

  /* get the maximum work group size for the kernel. */
  ret = clGetKernelWorkGroupInfo(S->kern, S->dev, CL_KERNEL_WORK_GROUP_SIZE,
                                 sizeof(size_t), &S->wgsize, NULL);
  if (ret != CL_SUCCESS)
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

/* search_set_outputs(): set the number of function outputs
 * interrogated by a search structure.
 *
 * arguments:
 *  @S: search structure pointer.
 *  @num: number of outputs.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int search_set_outputs (search_t *S, const unsigned int num) {
  /* check the input arguments. */
  if (!S || !num)
    return 0;

  /* store the new output count. */
  S->K = num;
  return 1;
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
  /* declare required variables:
   *  @idx: grid iteration index.
   *  @sz: grid dimension sizes.
   *  @gx: grid iteration point.
   */
  unsigned int N, Nrem;
  unsigned int *idx, *sz;
  vector_t *gx;

  /* check the input structure pointers. */
  if (!S || !x)
    return 0;

  /* refresh the calculation buffers. */
  if (!refresh_buffers(S))
    return 0;

  /* fill the non-grid data buffers on the host side. */
  if (!fill_buffers(S))
    return 0;

  /* write all data buffers to the compute device. */
  if (!write_buffers(S))
    return 0;

  /* set the kernel arguments. */
  if (!set_arguments(S))
    return 0;

  /* allocate the grid iteration variables. */
  if (!grid_iterator_alloc(S->grid, NULL, &idx, &sz, &gx))
    return 0;

  /* initialize the maximum variance. */
  S->vmax = 0.0;

  /* loop until no tasks remain. */
  Nrem = S->G;
  while (Nrem) {
    /* determine the task size. */
    N = (Nrem > S->N ? S->N : Nrem);

    /* fill the required amount of grid array elements. */
    for (unsigned int i = 0; i < N; i++) {
      /* copy the grid point into the array. */
      for (unsigned int d = 0; d < S->D; d++)
        S->xgrid[i * S->D + d] = vector_get(gx, d);

      /* move to the next grid point. */
      grid_iterator_next(S->grid, idx, sz, gx);
    }

    /* write xgrid to the device. */
    int ret = clEnqueueWriteBuffer(S->queue, S->dev_xgrid, CL_TRUE, 0,
                                   S->sz_xgrid, S->xgrid, 0, NULL, NULL);

    /* check for write failures. */
    if (ret != CL_SUCCESS)
      return 0;

    /* determine the total number of work items. */
    size_t Ntask = 1;
    while (Ntask < N)
      Ntask *= S->wgsize;

    /* enqueue the kernel. */
    ret = clEnqueueNDRangeKernel(S->queue, S->kern, 1, NULL,
                                 &Ntask, &S->wgsize,
                                 0, NULL, NULL);

    /* check for queueing failures. */
    if (ret != CL_SUCCESS)
      return 0;

    /* block until the kernel has completed. */
    clFinish(S->queue);

    /* read the results from the device. */
    if (!read_buffers(S))
      return 0;

    /* loop over the array of computed variances. */
    cl_double *xi = S->xgrid;
    for (unsigned int i = 0; i < N; i++, xi += S->D) {
      /* check if the current variance is larger. */
      if (S->var[i] > S->vmax) {
        /* copy the location of the larger variance. */
        memcpy(S->xmax, xi, S->sz_xmax);
        S->vmax = S->var[i];
      }
    }

    /* update the remaining task count. */
    Nrem -= N;
  }

  /* free the grid iteration variables. */
  grid_iterator_free(idx, sz, gx);

  /* store the identified location in the output vector. */
  for (unsigned int d = 0; d < S->D; d++)
    vector_set(x, d, S->xmax[d]);

  /* return success. */
  return 1;
}

