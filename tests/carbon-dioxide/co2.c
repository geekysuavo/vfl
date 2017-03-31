
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
  data_t *dat = data_alloc_from_file("co2.dat");

  /* adjust the observation locations for simpler fitting. */
  for (unsigned int i = 0; i < dat->N; i++)
    dat->data[i].x->data[0] -= 1995.0;

  /* set up a regression model. */
  model_t *mdl = model_alloc(model_type_vfr);
  model_set_alpha0(mdl, 100.0);
  model_set_beta0(mdl, 100.0);
  model_set_nu(mdl, 1.0e-6);
  model_set_data(mdl, dat);

  /* add cosine factors to the model. */
  model_add_factor(mdl, factor_cosine( 0.0,    1.0e5));
  model_add_factor(mdl, factor_cosine( 0.01, 100.0));
  model_add_factor(mdl, factor_cosine( 0.1,   10.0));
  model_add_factor(mdl, factor_cosine( 1.0,   10.0));
  model_add_factor(mdl, factor_cosine( 2.0,   10.0));
  model_add_factor(mdl, factor_cosine( 3.0,   10.0));
  model_add_factor(mdl, factor_cosine( 4.0,   10.0));
  model_add_factor(mdl, factor_cosine( 5.0,    0.1));
  model_add_factor(mdl, factor_cosine(10.0,    0.1));

  /* optimize. */
  optim_t *opt = optim_fg(mdl);
  opt->max_iters = 2000;
  opt->l0 = 0.0001;
  optim_execute(opt);

  /* allocate datasets for prediction. */
  double grid_values[] = { -25.0, 1.0e-2, 75.0 };
  matrix_view_t grid = matrix_view_array(grid_values, 1, 3);
  data_t *mean = data_alloc_from_grid(1, &grid);
  data_t *var = data_alloc_from_grid(1, &grid);

  /* compute the prediction. */
  model_predict_all(mdl, mean, var);

  /* reverse the adjustment made to the observation locations. */
  for (unsigned int i = 0; i < mean->N; i++) {
    mean->data[i].x->data[0] += 1995.0;
    var->data[i].x->data[0]  += 1995.0;
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

