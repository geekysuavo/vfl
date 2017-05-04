
/* include the vfl header. */
#include <vfl/vfl.h>

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
  data_t *dat = data_alloc_from_file("sinc.dat");

  /* set up a regression model. */
  model_t *mdl = model_alloc(vfl_model_vfr);
  model_set_alpha0(mdl, 1000.0);
  model_set_beta0(mdl, 2.5);
  model_set_nu(mdl, 1.0e-3);
  model_set_data(mdl, dat);

  /* add factors to the model. */
  const unsigned int M = 10;
  for (unsigned int i = 0; i < M; i++) {
    factor_t *f = factor_alloc(vfl_factor_impulse);
    factor_set(f, 0, 0.0);
    factor_set(f, 1, 0.01);
    model_add_factor(mdl, f);
  }

  /* randomly initialize the factor means. */
  for (unsigned int j = 0; j < mdl->M; j++)
    factor_set(mdl->factors[j], 0, 2.5 * rng_normal(R));

  /* optimize. */
  optim_t *opt = optim_alloc(vfl_optim_fg);
  optim_set_model(opt, mdl);
  optim_set_lipschitz_init(opt, 0.0001);
  optim_execute(opt);

  /* allocate datasets for prediction. */
  double grid_values[] = { -10.0, 0.001, 10.0 };
  matrix_view_t grid = matrix_view_array(grid_values, 1, 3);
  data_t *mean = data_alloc_from_grid(1, &grid);
  data_t *var = data_alloc_from_grid(1, &grid);

  /* compute the prediction. */
  model_predict_all(mdl, mean, var);

  /* output the prediction. */
  data_fwrite(mean, "mean.dat");
  data_fwrite(var, "var.dat");

  /* free the structures. */
  obj_free((object_t*) opt);
  obj_free((object_t*) mdl);
  obj_free((object_t*) mean);
  obj_free((object_t*) var);
  obj_free((object_t*) dat);
  obj_free((object_t*) R);

  /* return success. */
  return 0;
}

