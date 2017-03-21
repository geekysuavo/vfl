
/* include the required headers. */
#include <vfl/optim.h>

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

  /* read the dataset file. */
  data_t *dat = data_alloc_from_file("sinc.dat");

  /* set up a regression model. */
  model_t *mdl = model_vfr(1000.0, 10.0, 1.0e-3);
  model_set_data(mdl, dat);

  /* add factors to the model. */
  for (unsigned int i = 0; i < dat->N; i++) {
    const double xi = vector_get(dat->data[i].x, 0);
    factor_t *f = factor_fixed_impulse(xi, 0.001);
    model_add_factor(mdl, f);
  }

  /* initialize the factor precisions. */
  for (unsigned int j = 0; j < mdl->M; j++)
    factor_set(mdl->factors[j], 0, 1.0);

  /* optimize. */
  optim_t *opt = optim_fg(mdl);
  opt->l0 = 0.001;
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
  optim_free(opt);
  model_free(mdl);
  data_free(mean);
  data_free(var);
  data_free(dat);

  /* return success. */
  return 0;
}

