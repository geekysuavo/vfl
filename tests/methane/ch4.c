
/* include the required headers. */
#include <vfl/optim.h>

/* cosines: table of initial cosine factor parameters.
 */
static const struct {
  double mu, tau;
} cosines[] = {
  {  0.0,    1.0e5 },
  {  0.01, 100.0   },
  {  0.1,   10.0   },
  {  1.0,   10.0   },
  {  0.5,    1.0   },
  {  6.0,    1.0   },
  { 12.0,    1.0   },
  { 18.0,    0.1   },
  {  0.0,    0.0   } /* end marker. */
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
  data_t *dat = data_alloc_from_file("ch4.dat");

  /* adjust the observation locations for simpler fitting. */
  for (unsigned int i = 0; i < dat->N; i++)
    dat->data[i].x->data[0] -= 2000.0;

  /* set up a regression model. */
  model_t *mdl = model_alloc(model_type_vfr);
  model_set_alpha0(mdl, 100.0);
  model_set_beta0(mdl, 100.0);
  model_set_nu(mdl, 1.0e-6);
  model_set_data(mdl, dat);

  /* add cosine factors to the model. */
  for (unsigned int j = 0; cosines[j].tau; j++) {
    factor_t *f = factor_alloc(factor_type_cosine);
    factor_set(f, 0, cosines[j].mu);
    factor_set(f, 1, cosines[j].tau);
    model_add_factor(mdl, f);
  }

  /* optimize. */
  optim_t *opt = optim_alloc(optim_type_fg);
  optim_set_model(opt, mdl);
  optim_set_max_iters(opt, 2000);
  optim_set_lipschitz_init(opt, 0.00001);
  optim_execute(opt);

  /* allocate datasets for prediction. */
  double grid_values[] = { -20.0, 1.0e-2, 70.0 };
  matrix_view_t grid = matrix_view_array(grid_values, 1, 3);
  data_t *mean = data_alloc_from_grid(1, &grid);
  data_t *var = data_alloc_from_grid(1, &grid);

  /* compute the prediction. */
  model_predict_all(mdl, mean, var);

  /* reverse the adjustment made to the observation locations. */
  for (unsigned int i = 0; i < mean->N; i++) {
    mean->data[i].x->data[0] += 2000.0;
    var->data[i].x->data[0]  += 2000.0;
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

