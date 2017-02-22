
/* include the required headers. */
#include <vfl/optim.h>
#include <vfl/util/rng.h>

/* main(): application entry point.
 *
 * arguments:
 *  @argc: number of command-line argument strings.
 *  @argv: array of command-line argument strings.
 *
 * returns:
 *  application execution return code.
 */
int main (int argc, char **argv) {
  /* silly hack to avoid warnings. */
  if (!argc || !argv)
    return 1;

  /* allocate a random number generator. */
  rng_t *R = rng_alloc();

  /* read the dataset file. */
  data_t *dat = data_alloc();
  data_fread(dat, "ripley.dat");

  /* set up a classification model. */
  model_t *mdl = model_vfc(1.0e-6);
  model_set_data(mdl, dat);

  /* add factors to the model. */
  const unsigned int M = 10;
  for (unsigned int j = 0; j < M; j++) {
    /* create the independent factors along each dimension. */
    factor_t *fx = factor_impulse(0.0, 10.0);
    factor_t *fy = factor_impulse(0.0, 10.0);

    /* create the combined factor. */
    factor_t *f = factor_product(2, 0, fx, 1, fy);
    model_add_factor(mdl, f);
  }

  /* randomly initialize the factor means. */
  for (unsigned int j = 0; j < mdl->M; j++) {
    factor_set(mdl->factors[j], 0, -0.25 + 0.5 * rng_normal(R));
    factor_set(mdl->factors[j], 2,  0.45 + 0.5 * rng_normal(R));
  }

  /* set up an optimizer. */
  optim_t *opt = optim_fg(mdl);
  opt->l0 = 1.0;
  opt->dl = 0.1;
  unsigned int iter;
  double bound, bprev;
  bound = -1.0e99;

  /* perform a few optimization iterations. */
  for (iter = 0; iter <= 50; iter++) {
    bprev = bound;
    bound = model_bound(mdl);
    if (bound < bprev) break;

    fprintf(stderr, "%u %le", iter, bound);
    for (unsigned int j = 0; j < mdl->M; j++)
      fprintf(stderr, " %le %le",
        vector_get(mdl->factors[j]->par, 0),
        vector_get(mdl->factors[j]->par, 2));
    fprintf(stderr, "\n");
    fflush(stderr);

    if (!opt->iterate(opt)) break;
  }

  /* allocate datasets for prediction. */
  double grid_values[] = { -1.5, 0.01, 1.0,
                           -0.3, 0.01, 1.2 };
  matrix_view_t grid = matrix_view_array(grid_values, 2, 3);
  data_t *mean = data_alloc_from_grid(&grid);
  data_t *var = data_alloc_from_grid(&grid);

  /* compute the prediction. */
  model_predict_all(mdl, mean, var);
  data_fwrite(mean, "mean.dat");
  data_fwrite(var, "var.dat");

  /* free the structures. */
  model_free(mdl);
  data_free(mean);
  data_free(var);
  data_free(dat);
  rng_free(R);

  /* return success. */
  return 0;
}

