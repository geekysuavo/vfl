
/* include the required headers. */
#include <vfl/optim.h>

/* decays: table of initial decay factor parameters.
 */
static const struct {
  double alpha, beta;
} decays[] = {
  { 10.0,     1.0 },
  { 10.0,    10.0 },
  { 10.0,   100.0 },
  { 10.0,  1000.0 },
  { 10.0, 10000.0 },
  {  0.0,     0.0 } /* end marker. */
};

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
  data_t *dat = data_alloc_from_file("multexp.dat");

  /* set up a regression model. */
  model_t *mdl = model_alloc(model_type_vfr);
  model_set_alpha0(mdl, 10.0);
  model_set_beta0(mdl, 40.0);
  model_set_nu(mdl, 1.0e-3);
  model_set_data(mdl, dat);

  /* add factors to the model. */
  for (unsigned int j = 0; decays[j].alpha; j++) {
    factor_t *f = factor_alloc(factor_type_decay);
    factor_set(f, 0, decays[j].alpha);
    factor_set(f, 1, decays[j].beta);
    model_add_factor(mdl, f);
  }
  
  /* optimize. */
  optim_t *opt = optim_alloc(optim_type_fg, mdl);
  opt->l0 = 0.001;
  optim_execute(opt);

  /* allocate datasets for prediction. */
  double grid_values[] = { 0.0, 0.1, 150.0 };
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

