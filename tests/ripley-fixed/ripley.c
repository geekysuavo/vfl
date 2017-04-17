
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
  data_t *dat = data_alloc_from_file("ripley.dat");

  /* set up a classification model. */
  model_t *mdl = model_alloc(vfl_model_vfc);
  model_set_nu(mdl, 1.0e-6);
  model_set_data(mdl, dat);

  /* add factors to the model. */
  for (unsigned int i = 0; i < dat->N; i += 10) {
    /* create a factor along the x-dimension. */
    factor_t *fx = factor_alloc(vfl_factor_fixed_impulse);
    const double x = vector_get(dat->data[i].x, 0);
    fixed_impulse_set_location(fx, x);
    factor_set(fx, 0, 0.1);

    /* create a factor along the y-dimension. */
    factor_t *fy = factor_alloc(vfl_factor_fixed_impulse);
    const double y = vector_get(dat->data[i].x, 1);
    fixed_impulse_set_location(fy, y);
    factor_set(fy, 0, 0.1);

    /* add the combined factor. */
    factor_t *f = factor_alloc(vfl_factor_product);
    product_add_factor(f, 0, fx);
    product_add_factor(f, 1, fy);
    model_add_factor(mdl, f);

    /* initialize the factor parameters. */
    factor_set(mdl->factors[mdl->M - 1], 0, 10.0);
    factor_set(mdl->factors[mdl->M - 1], 1, 10.0);
  }

  /* optimize. */
  optim_t *opt = optim_alloc(vfl_optim_fg);
  optim_set_model(opt, mdl);
  optim_set_max_iters(opt, 50);
  optim_set_lipschitz_init(opt, 0.001);
  optim_execute(opt);

  /* allocate datasets for prediction. */
  double grid_values[] = { -1.5, 0.01, 1.0,
                           -0.3, 0.01, 1.2 };
  matrix_view_t grid = matrix_view_array(grid_values, 2, 3);
  data_t *mean = data_alloc_from_grid(1, &grid);
  data_t *var = data_alloc_from_grid(1, &grid);

  /* compute the prediction. */
  model_predict_all(mdl, mean, var);
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

