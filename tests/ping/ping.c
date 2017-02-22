
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
  data_fread(dat, "ping.dat");

  /* set up a regression model. */
  model_t *mdl = model_vfr(100.0, 100.0, 1.0e-6);
  model_set_data(mdl, dat);

  /* add a single factor to the model. */
  model_add_factor(mdl, factor_product(2, 0, factor_decay(200.0, 1000.0),
                                          0, factor_cosine(0.0, 0.1)));

  /* randomly adjust the frequency mean. */
  factor_set(mdl->factors[0], 2, 100.0 * rng_normal(R));

  /* set up an optimizer. */
  optim_t *opt = optim_fg(mdl);
  opt->l0 = 1.0;
  opt->dl = 0.1;
  unsigned int iter;
  double bound;

  /* perform a few optimization iterations. */
  for (iter = 0; iter < 1000; iter++) {
    bound = model_bound(mdl);
    fprintf(stderr, "%u %le", iter, bound);
    int mod = opt->iterate(opt);
    for (unsigned int p = 0; p < mdl->factors[0]->P; p++)
      fprintf(stderr, " %16.9le", vector_get(mdl->factors[0]->par, p));
    fprintf(stderr, "\n");
    if (!mod) break;
  }

  /* allocate datasets for prediction. */
  double grid_values[] = { 0.0, 1.0e-3, 10.0 };
  matrix_view_t grid = matrix_view_array(grid_values, 1, 3);
  data_t *mean = data_alloc_from_grid(&grid);
  data_t *var = data_alloc_from_grid(&grid);

  /* compute the prediction. */
  model_predict_all(mdl, mean, var);

  /* output the prediction. */
  data_fwrite(mean, "mean.dat");
  data_fwrite(var, "var.dat");

  /* free the structures. */
  optim_free(opt);
  model_free(mdl);
  data_free(mean);
  data_free(var);
  data_free(dat);
  rng_free(R);

  /* return success. */
  return 0;
}

