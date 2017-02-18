
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
  data_t *dat = data_alloc();
  data_fread(dat, "gauss.dat");

  /* set up a regression model. */
  model_t *mdl = model_vfr(100.0, 100.0, 1.0e-2);
  model_set_data(mdl, dat);

  /* add factors to the model. */
  model_add_factor(mdl, factor_decay(10.0, 1000.0));
  model_add_factor(mdl, factor_impulse(60.0, 0.01));
  model_add_factor(mdl, factor_impulse(180.0, 0.01));
  
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
    fprintf(stderr, "\n");
    if (!mod) break;
  }

  /* allocate datasets for prediction. */
  double grid_values[] = { 0.0, 1.0, 300.0 };
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

  /* return success. */
  return 0;
}
