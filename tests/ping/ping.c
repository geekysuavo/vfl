
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
  data_t *dat = data_alloc_from_file("ping.dat");

  /* set up a regression model. */
  model_t *mdl = model_alloc(vfl_model_vfr);
  model_set_alpha0(mdl, 100.0);
  model_set_beta0(mdl, 100.0);
  model_set_nu(mdl, 1.0e-6);
  model_set_data(mdl, dat);

  /* create a decay factor. */
  factor_t *fd = factor_alloc(vfl_factor_decay);
  factor_set(fd, 0,  200.0);
  factor_set(fd, 1, 1000.0);

  /* create a cosine factor. */
  factor_t *fc = factor_alloc(vfl_factor_cosine);
  factor_set(fc, 0, 0.0);
  factor_set(fc, 1, 0.1);

  /* add a factor as the product of the decay and cosine. */
  factor_t *f = factor_alloc(vfl_factor_product);
  product_add_factor(f, 0, fd);
  product_add_factor(f, 0, fc);
  model_add_factor(mdl, f);

  /* randomly adjust the frequency mean. */
  factor_set(mdl->factors[0], 2, 100.0 * rng_normal(R));

  /* optimize. */
  optim_t *opt = optim_alloc(vfl_optim_fg);
  optim_set_model(opt, mdl);
  optim_execute(opt);

  /* allocate datasets for prediction. */
  double grid_values[] = { 0.0, 1.0e-3, 10.0 };
  matrix_view_t grid = matrix_view_array(grid_values, 1, 3);
  data_t *mean = data_alloc_from_grid(1, &grid);
  data_t *var = data_alloc_from_grid(1, &grid);

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

