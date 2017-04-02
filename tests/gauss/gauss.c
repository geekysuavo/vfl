
/* include the required headers. */
#include <vfl/optim.h>

/* parms: table of initial factor parameters.
 */
static struct {
  double pa, pb;
} parms[] = {
  {  10.0, 1000.0  },
  {  60.0,    0.01 },
  { 180.0,    0.01 },
  {   0.0,    0.0  } /* end marker. */
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
  data_t *dat = data_alloc_from_file("gauss.dat");

  /* set up a regression model. */
  model_t *mdl = model_alloc(model_type_vfr);
  model_set_alpha0(mdl, 100.0);
  model_set_beta0(mdl, 100.0);
  model_set_nu(mdl, 1.0e-2);
  model_set_data(mdl, dat);

  /* add factors to the model. */
  for (unsigned int j = 0; parms[j].pb; j++) {
    factor_t *f = factor_alloc(j ? factor_type_impulse : factor_type_decay);
    factor_set(f, 0, parms[j].pa);
    factor_set(f, 1, parms[j].pb);
    model_add_factor(mdl, f);
  }
  
  /* optimize. */
  optim_t *opt = optim_alloc(optim_type_fg, mdl);
  optim_execute(opt);

  /* allocate datasets for prediction. */
  double grid_values[] = { 0.0, 1.0, 300.0 };
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

