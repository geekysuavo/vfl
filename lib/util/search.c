
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
"                            const __global float *x1,"             "\n" \
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
"  float *xs = xgrid[gid * D];"                                     "\n" \
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
"      float *xi = xdat[i * D];"                                    "\n" \
"      unsigned int pi = pdat[i];"                                  "\n" \
""                                                                  "\n" \
"      /* loop over the second matrix dimension. */"                "\n" \
"      for (unsigned int j = 0; j < n; j++, cidx++) {"              "\n" \
"        /* get the second data value. */"                          "\n" \
"        float *xj = xdat[j * D];"                                  "\n" \
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
  if (!mdl || !dat)
    return NULL;

  /* allocate a new structure pointer. */
  search_t *S = malloc(sizeof(search_t));
  if (!S)
    return NULL;

  /* store the model and dataset. */
  S->mdl = mdl;
  S->dat = dat;

  /* initialize the opencl variables. */
  S->dev = NULL;
  S->ctx = NULL;
  S->queue = NULL;
  S->prog = NULL;
  S->kern = NULL;
  S->src = NULL;

  /* initialize the host variables. */
  S->xgrid = S->xdat = S->xmax = S->par = S->C = NULL;
  S->pdat = NULL;
  S->vmax = 0.0;

  /* initialize the device memory variables. */
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

  /* connect to the first available compute device. */
  int ret = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &S->dev, NULL);
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

  /* FIXME: implement search_alloc() */

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

  /* FIXME: implement search_free() */

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

  /* FIXME: implement search_execute() */

  /* return success. */
  return 1;
}

