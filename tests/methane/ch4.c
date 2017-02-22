
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
  data_fread(dat, "ch4.dat");

  /* adjust the observation locations for simpler fitting. */
  for (unsigned int i = 0; i < dat->N; i++)
    matrix_set(dat->X, i, 0, matrix_get(dat->X, i, 0) - 2000.0);

  /* set up a regression model. */
  model_t *mdl = model_vfr(100.0, 100.0, 1.0e-6);
  model_set_data(mdl, dat);

  /* add cosine factors to the model. */
  model_add_factor(mdl, factor_cosine( 0.0,    1.0e5));
  model_add_factor(mdl, factor_cosine( 0.01, 100.0));
  model_add_factor(mdl, factor_cosine( 0.1,   10.0));
  model_add_factor(mdl, factor_cosine( 1.0,   10.0));
  model_add_factor(mdl, factor_cosine( 0.5,    1.0));
  model_add_factor(mdl, factor_cosine( 6.0,    1.0));
  model_add_factor(mdl, factor_cosine(12.0,    1.0));
  model_add_factor(mdl, factor_cosine(18.0,    0.1));

  /* set up an optimizer. */
  optim_t *opt = optim_fg(mdl);
  opt->l0 = 0.00001;
  opt->dl = 0.1;
  unsigned int iter;
  double bound;

  /* perform a few optimization iterations. */
  for (iter = 0; iter < 2000; iter++) {
    bound = model_bound(mdl);
    fprintf(stderr, "%u %le\n", iter, bound);
    if (!opt->iterate(opt))
      break;
  }

  /* allocate datasets for prediction. */
  double grid_values[] = { -20.0, 1.0e-2, 70.0 };
  matrix_view_t grid = matrix_view_array(grid_values, 1, 3);
  data_t *mean = data_alloc_from_grid(&grid);
  data_t *var = data_alloc_from_grid(&grid);

  /* compute the prediction. */
  model_predict_all(mdl, mean, var);

  /* reverse the adjustment made to the observation locations. */
  for (unsigned int i = 0; i < mean->N; i++) {
    matrix_set(mean->X, i, 0, matrix_get(mean->X, i, 0) + 2000.0);
    matrix_set(var->X,  i, 0, matrix_get(var->X,  i, 0) + 2000.0);
  }

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

