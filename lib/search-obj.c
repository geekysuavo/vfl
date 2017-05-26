
/* include the search header. */
#include <vfl/search.h>

/* include the required object headers. */
#include <vfl/base/int.h>
#include <vfl/base/map.h>
#include <vfl/base/list.h>

/* private search functions: */
void free_buffers (search_t *S);
void free_kernel (search_t *S);

/* search_init(): initialize the contents of a search structure.
 *
 * arguments:
 *  @S: search structure pointer.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
static int search_init (search_t *S) {
  /* initialize the model, dataset and grid. */
  S->grid = NULL;
  S->mdl = NULL;
  S->dat = NULL;

  /* initialize the buffer sizes. */
  S->D = S->P = S->K = S->N = S->n = 0;

#ifdef __VFL_USE_OPENCL
  /* initialize the opencl variables. */
  S->plat = NULL;
  S->dev = NULL;
  S->ctx = NULL;
  S->queue = NULL;
  S->prog = NULL;
  S->kern = NULL;
  S->src = NULL;

  /* initialize the device-side pointers. */
  S->dev_par = S->dev_var = S->dev_xgrid = NULL;
  S->dev_xdat = S->dev_pdat = S->dev_C = NULL;
  S->dev_cblk = NULL;

  /* initialize the host-side pointers. */
  S->par = S->var = S->xgrid = S->xmax = S->xdat = S->C = NULL;
  S->pdat = NULL;
#else
  S->cs = NULL;
#endif
  S->cov = NULL;
  S->vmax = 0.0;

  /* return success. */
  return 1;
}

/* search_free(): free the contents of a search structure.
 *
 * arguments:
 *  @S: search structure pointer to free.
 */
static void search_free (search_t *S) {
  /* free the the kernel and calculation buffers. */
  free_buffers(S);
  free_kernel(S);
}

/* --- */

/* search_getprop_model(): method for getting search models.
 *  - see object_getprop_fn() for details.
 */
static object_t *search_getprop_model (const search_t *S) {
  /* return nothing if the model is null. */
  if (!S->mdl)
    VFL_RETURN_NIL;

  /* directly return the model structure pointer. */
  return (object_t*) S->mdl;
}

/* search_getprop_data(): method for getting search datasets.
 *  - see object_getprop_fn() for details.
 */
static object_t *search_getprop_data (const search_t *S) {
  /* return nothing if the dataset is null. */
  if (!S->dat)
    VFL_RETURN_NIL;

  /* directly return the dataset structure pointer. */
  return (object_t*) S->dat;
}

/* search_getprop_grid(): method for getting search grids.
 *  - see object_getprop_fn() for details.
 */
static object_t *search_getprop_grid (const search_t *S) {
  /* return nothing if the grid is null. */
  if (!S->grid)
    VFL_RETURN_NIL;

  /* return the gridding matrix as a new list. */
  return (object_t*) list_alloc_from_matrix(S->grid);
}

/* search_getprop_outputs(): method for getting search output counts.
 *  - see object_getprop_fn() for details.
 */
static int_t *search_getprop_outputs (const search_t *S) {
  /* return the output count as a new integer. */
  return int_alloc_with_value(S->P);
}

/* search_setprop_model(): method for setting search models.
 *  - see object_setprop_fn() for details.
 */
static int search_setprop_model (search_t *S, object_t *val) {
  /* admit only model values. */
  if (!OBJECT_IS_MODEL(val))
    return 0;

  /* set the model and return. */
  return search_set_model(S, (model_t*) val);
}

/* search_setprop_data(): method for setting search datasets.
 *  - see object_setprop_fn() for details.
 */
static int search_setprop_data (search_t *S, object_t *val) {
  /* admit only dataset values. */
  if (!OBJECT_IS_DATA(val))
    return 0;

  /* set the dataset and return. */
  return search_set_data(S, (data_t*) val);
}

/* search_setprop_grid(): method for setting search grids.
 *  - see object_setprop_fn() for details.
 */
static int search_setprop_grid (search_t *S, object_t *val) {
  /* admit only list values. */
  if (!OBJECT_IS_LIST(val))
    return 0;

  /* cast the list to a matrix. */
  matrix_t *grid = list_to_matrix((list_t*) val);
  if (!grid)
    return 0;

  /* store the grid and return. */
  return search_set_grid(S, grid);
}

/* search_setprop_outputs(): method for setting search output counts.
 *  - see object_setprop_fn() for details.
 */
static int search_setprop_outputs (search_t *S, object_t *val) {
  /* admit only integer values. */
  if (!OBJECT_IS_INT(val))
    return 0;

  /* admit only positive integers. */
  const long ival = int_get((int_t*) val);
  if (ival <= 0)
    return 0;

  /* store the new value and return. */
  return search_set_outputs(S, (unsigned int) ival);
}

/* search_properties: array of accessible object properties.
 */
static object_property_t search_properties[] = {
  { "model",
    (object_getprop_fn) search_getprop_model,
    (object_setprop_fn) search_setprop_model
  },
  { "data",
    (object_getprop_fn) search_getprop_data,
    (object_setprop_fn) search_setprop_data
  },
  { "grid",
    (object_getprop_fn) search_getprop_grid,
    (object_setprop_fn) search_setprop_grid
  },
  { "outputs",
    (object_getprop_fn) search_getprop_outputs,
    (object_setprop_fn) search_setprop_outputs
  },
  { NULL, NULL, NULL }
};

/* --- */

/* search_method_execute(): method for running posterior variance searches.
 *  - see object_method_fn() for details.
 */
static object_t *search_method_execute (search_t *S, map_t *args) {
  /* get the current search dimensionality. */
  const unsigned int D = (S->mdl ? S->mdl->D : 0);
  if (!D)
    return NULL;

  /* allocate a temporary vector for the result. */
  vector_t *x = vector_alloc(D);
  if (!x)
    return NULL;

  /* execute the search. */
  if (!search_execute(S, x)) {
    vector_free(x);
    return NULL;
  }

  /* cast the vector into a list. */
  object_t *lst = list_alloc_from_vector(x);
  vector_free(x);
  if (!lst)
    return NULL;

  /* return the new list. */
  return lst;
}

/* search_methods: array of callable object methods.
 */
static object_method_t search_methods[] = {
  { "execute", (object_method_fn) search_method_execute },
  { NULL, NULL }
};

/* --- */

/* search_type: active learning search type structure.
 */
static object_type_t search_type = {
  "search",                                      /* name      */
  sizeof(search_t),                              /* size      */

  (object_init_fn) search_init,                  /* init      */
  NULL,                                          /* copy      */
  (object_free_fn) search_free,                  /* free      */
  NULL,                                          /* test      */
  NULL,                                          /* cmp       */

  NULL,                                          /* add       */
  NULL,                                          /* sub       */
  NULL,                                          /* mul       */
  NULL,                                          /* div       */
  NULL,                                          /* pow       */

  NULL,                                          /* get       */
  NULL,                                          /* set       */
  search_properties,                             /* props     */
  search_methods                                 /* methods   */
};

/* vfl_object_search: address of the search_type structure. */
const object_type_t *vfl_object_search = &search_type;

