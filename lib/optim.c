
/* include the optimizer header. */
#include <vfl/optim.h>

/* optim_alloc(): allocate a new optimizer.
 *
 * arguments:
 *  @mdl: model structure pointer.
 *  @bytes: amount of memory to allocate, or zero for the default.
 *
 * returns:
 *  newly allocated and initialized optimizer structure pointer.
 */
optim_t *optim_alloc (model_t *mdl, const unsigned int bytes) {
  /* return null if the model is null or incapable of inference. */
  if (!mdl || !model_infer(mdl))
    return NULL;

  /* determine the amount of memory to allocate. */
  const unsigned int sz = (bytes ? bytes : sizeof(optim_t));

  /* allocate the structure pointer. */
  optim_t *opt = (optim_t*) malloc(sz);
  if (!opt)
    return NULL;

  /* store the core members. */
  opt->bytes = sz;
  opt->mdl = mdl;

  /* initialize the function pointers. */
  opt->free = NULL;

  /* determine the maximum parameter count of the model factors. */
  unsigned int pmax = 0;
  for (unsigned int j = 0; j < mdl->M; j++) {
    const unsigned int pj = mdl->factors[j]->P;
    if (pj > pmax)
      pmax = pj;
  }

  /* allocate the iteration vectors. */
  opt->xa = vector_alloc(pmax);
  opt->xb = vector_alloc(pmax);
  opt->x = vector_alloc(pmax);
  opt->g = vector_alloc(pmax);

  /* allocate the temporaries. */
  opt->Fs = matrix_alloc(pmax, pmax);

  /* check that allocation was successful. */
  if (!opt->xa || !opt->xb || !opt->x || !opt->g || !opt->Fs) {
    optim_free(opt);
    return NULL;
  }

  /* initialize the search parameters. */
  opt->L0 = 1.0;

  /* return the new optimizer. */
  return opt;
}

/* optim_free(): free an allocated optimizer.
 *
 * arguments:
 *  @opt: optimizer structure pointer to free.
 */
void optim_free (optim_t *opt) {
  /* return if the structure pointer is null. */
  if (!opt)
    return;

  /* if the optimizer has a free function assigned, execute it. */
  if (opt->free)
    opt->free(opt);

  /* free the iteration vectors. */
  vector_free(opt->xa);
  vector_free(opt->xb);
  vector_free(opt->x);
  vector_free(opt->g);

  /* free the temporaries. */
  matrix_free(opt->Fs);

  /* free the structure pointer. */
  free(opt);
}

