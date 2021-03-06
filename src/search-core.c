
/* include the vfl header. */
#include <vfl/vfl.h>
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
 *  @cblk: array of N*n floats holding the kernel vector elements.
 *  @var: array of N floats holding the output variance calculations.
 */

/* SEARCH_MAX_GRID: maximum number of grid points searched at once.
 */
#define SEARCH_MAX_GRID 512

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
"                            __global double *cblk,"                "\n" \
"                            __global double *var) {"               "\n" \
"  /* get the grid index. */"                                       "\n" \
"  const size_t gid = get_global_id(0);"                            "\n" \
""                                                                  "\n" \
"  /* avoid buffer overflow. */"                                    "\n" \
"  if (gid >= N)"                                                   "\n" \
"    return;"                                                       "\n" \
""                                                                  "\n" \
"  /* get the thread-owned grid location. */"                       "\n" \
"  const __global double *xs = xgrid + (gid * D);"                  "\n" \
""                                                                  "\n" \
"  /* get the thread-owned kernel vector. */"                       "\n" \
"  __global double *cv = cblk + (gid * n);"                         "\n" \
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
"      /* get the data value. */"                                   "\n" \
"      const __global double *xi = xdat + (i * D);"                 "\n" \
"      uint pi = pdat[i];"                                          "\n" \
""                                                                  "\n" \
"      /* compute the kernel vector element. */"                    "\n" \
"      cv[i] = vfl_covkernel(par, xs, xi, ps, pi, D);"              "\n" \
""                                                                  "\n" \
"      /* loop over the off-diagonal row elements. */"              "\n" \
"      for (uint j = 0; j < i; j++, cidx++)"                        "\n" \
"        sum -= 2.0 * cv[i] * cv[j] * C[cidx];"                     "\n" \
""                                                                  "\n" \
"      /* include the diagonal matrix element. */"                  "\n" \
"      sum -= cv[i] * cv[i] * C[cidx++];"                           "\n" \
"    }"                                                             "\n" \
"  }"                                                               "\n" \
""                                                                  "\n" \
"  /* store the computed result. */"                                "\n" \
"  var[gid] = sum;"                                                 "\n" \
"}\n"

/* * * * private function definitions: * * * */

/* set_kernel(): initialize the compute device kernel information
 * used by a search structure.
 *
 * arguments:
 *  @S: search structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
static int set_kernel (Search *S) {
#ifdef __VFL_USE_OPENCL
  /* generate the model kernel code. */
  char *ksrc = model_kernel(S->mdl);
  if (!ksrc)
    return 0;

  /* allocate memory for the complete program code. */
  const unsigned int len = strlen(SEARCH_FORMAT) + strlen(ksrc) + 8;
  S->src = malloc(len);
  if (!S->src) {
    free(ksrc);
    return 0;
  }

  /* write the complete program code and free the model kernel code. */
  sprintf(S->src, SEARCH_FORMAT, ksrc);
  free(ksrc);

  /* get the first available compute platform. */
  int ret = clGetPlatformIDs(1, &S->plat, NULL);
  if (ret != CL_SUCCESS)
    return 0;

  /* connect to the first available compute device. */
  ret = clGetDeviceIDs(S->plat, CL_DEVICE_TYPE_GPU, 1, &S->dev, NULL);
  if (ret != CL_SUCCESS)
    return 0;

  /* create a compute context. */
  S->ctx = clCreateContext(NULL, 1, &S->dev, NULL, NULL, &ret);
  if (!S->ctx)
    return 0;

  /* create a command queue. */
  S->queue = clCreateCommandQueue(S->ctx, S->dev, 0, &ret);
  if (!S->queue)
    return 0;

  /* create the compute program. */
  S->prog = clCreateProgramWithSource(S->ctx, 1, (const char**) &S->src,
                                      NULL, &ret);

  /* check for program creation failure. */
  if (!S->prog)
    return 0;

  /* build the program executable. */
  ret = clBuildProgram(S->prog, 0, NULL, NULL, NULL, NULL);
  if (ret != CL_SUCCESS)
    return 0;

  /* create the compute kernel. */
  S->kern = clCreateKernel(S->prog, "vfl_variance", &ret);
  if (!S->kern || ret != CL_SUCCESS)
    return 0;

  /* get the maximum work group size for the kernel. */
  ret = clGetKernelWorkGroupInfo(S->kern, S->dev, CL_KERNEL_WORK_GROUP_SIZE,
                                 sizeof(size_t), &S->wgsize, NULL);

  /* check for query failure. */
  if (ret != CL_SUCCESS)
    return 0;
#endif

  /* return success. */
  return 1;
}

/* set_arguments(): set the kernel arguments of a search structure.
 *
 * arguments:
 *  @S: search structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
static int set_arguments (Search *S) {
#ifdef __VFL_USE_OPENCL
  /* initialize the status code. */
  int ret = CL_SUCCESS;

  /* set all arguments. */
  ret |= clSetKernelArg(S->kern,  0, sizeof(cl_mem),  &S->dev_xgrid);
  ret |= clSetKernelArg(S->kern,  1, sizeof(cl_mem),  &S->dev_xdat);
  ret |= clSetKernelArg(S->kern,  2, sizeof(cl_mem),  &S->dev_pdat);
  ret |= clSetKernelArg(S->kern,  3, sizeof(cl_mem),  &S->dev_par);
  ret |= clSetKernelArg(S->kern,  4, sizeof(cl_mem),  &S->dev_C);
  ret |= clSetKernelArg(S->kern,  5, sizeof(cl_uint), &S->D);
  ret |= clSetKernelArg(S->kern,  6, sizeof(cl_uint), &S->K);
  ret |= clSetKernelArg(S->kern,  7, sizeof(cl_uint), &S->N);
  ret |= clSetKernelArg(S->kern,  8, sizeof(cl_uint), &S->n);
  ret |= clSetKernelArg(S->kern,  9, sizeof(cl_mem),  &S->dev_cblk);
  ret |= clSetKernelArg(S->kern, 10, sizeof(cl_mem),  &S->dev_var);

  /* return the resulting status code. */
  return (ret == CL_SUCCESS);
#else
  /* return success. */
  return 1;
#endif
}

/* free_kernel(): free all allocated compute platform information.
 *
 * arguments:
 *  @S: search structure pointer.
 */
void free_kernel (Search *S) {
#ifdef __VFL_USE_OPENCL
  /* release the opencl variables. */
  clReleaseProgram(S->prog);
  clReleaseKernel(S->kern);
  clReleaseCommandQueue(S->queue);
  clReleaseContext(S->ctx);

  /* reset the opencl variables. */
  S->prog = NULL;
  S->kern = NULL;
  S->queue = NULL;
  S->ctx = NULL;

  /* free the kernel source code string. */
  free(S->src);
  S->src = NULL;
#endif
}

/* free_buffers(): free all allocated calculation-related buffers
 * within a search structure.
 *
 * arguments:
 *  @S: search structure pointer.
 */
void free_buffers (Search *S) {
  /* free the dense covariance matrix. */
  matrix_free(S->cov);
  S->cov = NULL;

#ifdef __VFL_USE_OPENCL
  /* free the host-side calculation variables. */
  free(S->par);

  /* free the device-side memory objects. */
  clReleaseMemObject(S->dev_par);
  clReleaseMemObject(S->dev_var);
  clReleaseMemObject(S->dev_xgrid);
  clReleaseMemObject(S->dev_xdat);
  clReleaseMemObject(S->dev_pdat);
  clReleaseMemObject(S->dev_cblk);
  clReleaseMemObject(S->dev_C);

  /* reset the host-side pointers. */
  S->par = S->var = S->xgrid = S->xmax = S->xdat = S->C = NULL;
  S->pdat = NULL;

  /* reset the device-side pointers. */
  S->dev_par = S->dev_var = S->dev_xgrid = NULL;
  S->dev_xdat = S->dev_pdat = S->dev_C = NULL;
  S->dev_cblk = NULL;
#else
  /* free the kernel vector. */
  vector_free(S->cs);
  S->cs = NULL;
#endif
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
static int refresh_buffers (Search *S) {
  /* get the new sizes. */
#ifdef __VFL_USE_OPENCL
  const size_t P = S->mdl->P + 1;
  const size_t D = S->mdl->D;
#endif
  const size_t n = S->dat->N;

  /* determine the total grid size. */
  size_t G;
  grid_iterator_alloc(S->grid, &G, NULL, NULL, NULL);

  /* check if the grid size exceeds the maximum grid size. */
  size_t N = G;
  if (N > SEARCH_MAX_GRID)
    N = SEARCH_MAX_GRID;

#ifdef __VFL_USE_OPENCL
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
    S->sz_cblk  = sizeof(cl_double) * N * n;
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

    S->dev_cblk = clCreateBuffer(S->ctx, CL_MEM_WRITE_ONLY,
                                 S->sz_cblk, NULL, NULL);

    /* allocate the device-side buffer: C. */
    S->dev_C = clCreateBuffer(S->ctx, CL_MEM_READ_ONLY,
                              S->sz_C, NULL, NULL);

    /* check for allocation failures. */
    if (!S->dev_par || !S->dev_var || !S->dev_xgrid ||
        !S->dev_xdat || !S->dev_pdat || !S->dev_cblk || !S->dev_C)
      return 0;

    /* store the new sizes. */
    S->D = D;
    S->P = P;
    S->G = G;
    S->N = N;
    S->n = n;
  }
#else
  /* check for any differences. */
  if (S->n != n) {
    /* free the buffers. */
    free_buffers(S);

    /* allocate the dense covariance matrix and kernel vector. */
    S->cov = matrix_alloc(n, n);
    S->cs = vector_alloc(n);
    if (!S->cov || !S->cs)
      return 0;

    /* store the new sizes. */
    S->G = G;
    S->N = N;
    S->n = n;
  }
#endif

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
static int fill_buffers (Search *S) {
#ifdef __VFL_USE_OPENCL
  /* store the current noise precision and weight ratio. */
  S->par[0] = 1.0 / (S->mdl->nu * S->mdl->tau);

  /* store the current factor parameters. */
  for (size_t j = 0, p0 = 1; j < S->mdl->M; j++) {
    /* get the current factor parameter vector. */
    const Vector *par = S->mdl->factors[j]->par;

    /* copy the vector elements into the local array. */
    for (size_t p = 0; p < par->len; p++)
      S->par[p0 + p] = vector_get(par, p);

    /* increment the array offset. */
    p0 += par->len;
  }
#endif

  /* compute the covariance matrix elements. */
  for (size_t i = 0; i < S->n; i++) {
    /* get the row-wise observation. */
    Datum *di = data_get(S->dat, i);

#ifdef __VFL_USE_OPENCL
    /* while we're here, store the data array values. */
    S->pdat[i] = di->p;
    for (size_t d = 0; d < S->D; d++)
      S->xdat[i * S->D + d] = vector_get(di->x, d);
#endif

    /* loop over the elements of each row. */
    for (size_t j = 0; j <= i; j++) {
      /* get the column-wise observation. */
      Datum *dj = data_get(S->dat, j);

      /* compute the covariance matrix element. */
      const double cij = model_cov(S->mdl, di->x, dj->x, di->p, dj->p);

      /* store the computed matrix element. */
      matrix_set(S->cov, i, j, cij);
      if (i != j)
        matrix_set(S->cov, j, i, cij);
    }
  }

  /* compute the current model->data fit error estimate. */
  VectorView z = vector_subvector(S->mdl->tmp, 0, S->mdl->K);
  blas_dtrmv(BLAS_TRANS, S->mdl->L, S->mdl->wbar, &z);
  const double wSw = blas_ddot(&z, &z);
  const double yy = data_inner(S->dat);
  const double alpha = S->mdl->alpha0 + (double) S->mdl->dat->N;
  const double beta = S->mdl->beta0 + (yy - wSw);
  const double tauinv = beta / alpha;

  /* add the noise estimate as jitter to the variances. */
  z = matrix_diag(S->cov);
  vector_add_const(&z, tauinv);

  /* compute the cholesky decomposition of the covariance matrix. */
  if (!chol_decomp(S->cov) || !chol_invert(S->cov, S->cov)) {
    /* output a warning message. */
    fprintf(stderr, "cov (%zux%zu) is singular!\n",
            S->cov->rows, S->cov->cols);
    fflush(stderr);

    /* return failure. */
    return 0;
  }

#ifdef __VFL_USE_OPENCL
  /* pack the inverted matrix into the host-side array. */
  for (size_t i = 0, cidx = 0; i < S->n; i++)
    for (size_t j = 0; j <= i; j++, cidx++)
      S->C[cidx] = matrix_get(S->cov, i, j);
#endif

  /* return success. */
  return 1;
}

/* write_buffers(): write the contents of all host-side calculation
 * buffers (except the grid buffer) to the compute device associated
 * with a search structure.
 *
 * arguments:
 *  @S: search structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
static int write_buffers (Search *S) {
#ifdef __VFL_USE_OPENCL
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
#else
  /* return success. */
  return 1;
#endif
}

/* write_grid(): write the contents of the host-side grid buffer
 * to the compute device associated with a search structure.
 *
 * arguments:
 *  @S: search structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
static int write_grid (Search *S) {
#ifdef __VFL_USE_OPENCL
  /* write xgrid to the device. */
  int ret = clEnqueueWriteBuffer(S->queue, S->dev_xgrid, CL_TRUE, 0,
                                 S->sz_xgrid, S->xgrid, 0, NULL, NULL);

  /* return the resulting status code. */
  return (ret == CL_SUCCESS);
#else
  /* return success. */
  return 1;
#endif
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
static int read_buffers (Search *S) {
#ifdef __VFL_USE_OPENCL
  /* read the results from the device. */
  int ret = clEnqueueReadBuffer(S->queue, S->dev_var, CL_TRUE, 0,
                                S->sz_var, S->var, 0, NULL, NULL);

  /* return the resulting status code. */
  return (ret == CL_SUCCESS);
#else
  /* return success. */
  return 1;
#endif
}

/* launch_kernel(): enqueue a set of kernel execution requests
 * to the compute device associated to a search structure.
 *
 * arguments:
 *  @S: search structure pointer.
 *  @Ntask: pointer to the task size.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
static int launch_kernel (Search *S, size_t *Ntask) {
#ifdef __VFL_USE_OPENCL
  /* enqueue the kernel. */
  int ret = clEnqueueNDRangeKernel(S->queue, S->kern, 1, NULL,
                                   Ntask, &S->wgsize,
                                   0, NULL, NULL);

  /* check for queueing failures. */
  if (ret != CL_SUCCESS)
    return 0;

  /* block until the kernel has completed. */
  clFinish(S->queue);
#endif

  /* return success. */
  return 1;
}

/* * * * function definitions: * * * */

/* search_set_model(): set the model emulated by a search structure.
 *
 * arguments:
 *  @S: search structure pointer.
 *  @mdl: new target model.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int search_set_model (Search *S, Model *mdl) {
  /* check the input pointers. */
  if (!S || !mdl)
    return 0;

  /* check if a model is already assigned to the search. */
  if (S->mdl) {
    /* release all variables tied to the model. */
    free_buffers(S);
    free_kernel(S);

    /* release the reference to the current model. */
    Py_DECREF(S->mdl);
  }

  /* store the new model. */
  Py_INCREF(mdl);
  S->mdl = mdl;

#ifndef __VFL_USE_OPENCL
  /* store the new parameter and dimension counts. */
  S->P = S->mdl->P + 1;
  S->D = S->mdl->D;
#endif

  /* attempt to set the kernel information. */
  return set_kernel(S);
}

/* search_set_data(): set the dataset modeled by a search structure.
 *
 * arguments:
 *  @S: search structure pointer.
 *  @dat: new input dataset.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int search_set_data (Search *S, Data *dat) {
  /* check the input pointers. */
  if (!S || !dat)
    return 0;

  /* release the reference to the current dataset. */
  Py_XDECREF(S->dat);

  /* store the new dataset and return success. */
  Py_INCREF(dat);
  S->dat = dat;
  return 1;
}

/* search_set_grid(): set the gridding matrix used by a search structure.
 *
 * arguments:
 *  @S: search structure pointer.
 *  @grid: new gridding matrix.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int search_set_grid (Search *S, Matrix *grid) {
  /* check the input pointers. */
  if (!S || !grid)
    return 0;

  /* check that the matrix contains a valid grid. */
  if (!grid_validate(grid))
    return 0;

  /* release the existing grid. */
  if (S->grid)
    matrix_free(S->grid);

  /* store the new grid and return success. */
  S->grid = grid;
  return 1;
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
int search_set_outputs (Search *S, size_t num) {
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
int search_execute (Search *S, Vector *x) {
  /* declare required variables:
   *  @idx: grid iteration index.
   *  @sz: grid dimension sizes.
   *  @gx: grid iteration point.
   */
  size_t N, Nrem;
  size_t *idx, *sz;
  Vector *gx;

  /* check the input structure pointers. */
  if (!S || !x)
    return 0;

  /* check that the search contains all required variables. */
  if (!S->grid || !S->mdl || !S->dat)
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

  /* initialize the maximum variance datum. */
  VectorView xview;
  Datum dmax;
  dmax.x = &xview;
  dmax.y = 0.0;
  dmax.p = 0;

  /* loop until no tasks remain. */
  Nrem = S->G;
  while (Nrem) {
    /* determine the task size. */
    N = (Nrem > S->N ? S->N : Nrem);

    /* fill the required amount of grid array elements. */
    for (size_t i = 0; i < N; i++) {
#ifdef __VFL_USE_OPENCL
      /* copy the grid point into the array. */
      for (size_t d = 0; d < S->D; d++)
        S->xgrid[i * S->D + d] = vector_get(gx, d);
#else
      /* initialize variables for checking for new maxima. */
      double sum = 0.0;
      dmax.x = gx;

      /* loop over the outputs. */
      for (size_t ps = 0; ps < S->K; ps++) {
        /* include the first term. */
        sum += model_cov(S->mdl, gx, gx, ps, ps);

        /* compute the kernel vector elements. */
        Datum *dj = S->dat->data;
        for (size_t j = 0; j < S->n; j++, dj++)
          vector_set(S->cs, j, model_cov(S->mdl, dj->x, gx, dj->p, ps));

        /* include the second term. */
        for (size_t j = 0; j < S->n; j++)
          for (size_t k = 0; k < S->n; k++)
            sum -= vector_get(S->cs, j)
                 * vector_get(S->cs, k)
                 * matrix_get(S->cov, j, k);
      }

      /* check if the current variance is larger. */
      if (sum > dmax.y && !data_find(S->dat, &dmax)) {
        /* copy the location of the greater variance. */
        vector_copy(x, gx);
        dmax.y = sum;
      }
#endif

      /* move to the next grid point. */
      grid_iterator_next(S->grid, idx, sz, gx);
    }

    /* write the grid to the device. */
    if (!write_grid(S))
      return 0;

    /* determine the total number of work items. */
    size_t Ntask = 1;
    while (Ntask < N)
      Ntask *= S->wgsize;

    /* execute the kernel over the current grid points. */
    if (!launch_kernel(S, &Ntask))
      return 0;

    /* read the results from the device. */
    if (!read_buffers(S))
      return 0;

#ifdef __VFL_USE_OPENCL
    /* loop over the array of computed variances. */
    cl_double *xi = S->xgrid;
    for (size_t i = 0; i < N; i++, xi += S->D) {
      /* initialize the datum to search for existing observations. */
      xview = vector_view_array(xi, S->D);

      /* check if the current variance is larger. */
      if (S->var[i] > dmax.y && !data_find(S->dat, &dmax)) {
        /* copy the location of the larger variance. */
        memcpy(S->xmax, xi, S->sz_xmax);
        dmax.y = S->var[i];
      }
    }
#endif

    /* update the remaining task count. */
    Nrem -= N;
  }

  /* free the grid iteration variables. */
  grid_iterator_free(idx, sz, gx);

#ifdef __VFL_USE_OPENCL
  /* store the identified location in the output vector. */
  for (size_t d = 0; d < S->D; d++)
    vector_set(x, d, S->xmax[d]);
#endif

  /* return success. */
  return 1;
}

