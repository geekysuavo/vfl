
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
  data_t *dat = data_alloc_from_file("cosines.dat");

  /* set up a fixed-tau regression model. */
  model_t *mdl = model_alloc(vfl_model_tauvfr);
  tauvfr_set_tau(mdl, 1.0);
  model_set_nu(mdl, 1.0e-6);
  model_set_data(mdl, dat);

  /* add factors to the model. */
  const unsigned int M = 4;
  for (unsigned int j = 0; j < M; j++) {
    factor_t *f = factor_alloc(vfl_factor_cosine);
    factor_set(f, 0, 0.0);
    factor_set(f, 1, 1.0e-5);
    model_add_factor(mdl, f);
  }

  /* randomly initialize the factor frequency means. */
  for (unsigned int j = 0; j < mdl->M; j++)
    factor_set(mdl->factors[j], 0, 300.0 * rng_normal(R));

  /* optimize. */
  optim_t *opt = optim_alloc(vfl_optim_fg);
  optim_set_model(opt, mdl);
  optim_set_lipschitz_init(opt, 0.001);
  optim_execute(opt);

  /* allocate datasets for prediction. */
  double grid_values[] = { 0.0, 1.0e-3, 0.5 };
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

