
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
  data_t *dat = data_alloc_from_file("ripley.dat");

  /* set up a classification model. */
  model_t *mdl = model_alloc(model_type_vfc);
  model_set_nu(mdl, 1.0e-6);
  model_set_data(mdl, dat);

  /* add factors to the model. */
  const unsigned int M = 10;
  for (unsigned int j = 0; j < M; j++) {
    /* create a factor along the x-dimension. */
    factor_t *fx = factor_alloc(factor_type_impulse);
    factor_set(fx, 0,  0.0);
    factor_set(fx, 1, 10.0);

    /* create a factor along the y-dimension. */
    factor_t *fy = factor_alloc(factor_type_impulse);
    factor_set(fy, 0,  0.0);
    factor_set(fy, 1, 10.0);

    /* add the combined factor. */
    factor_t *f = factor_alloc(factor_type_product);
    product_add_factor(f, 0, fx);
    product_add_factor(f, 1, fy);
    model_add_factor(mdl, f);
  }

  /* randomly initialize the factor means. */
  for (unsigned int j = 0; j < mdl->M; j++) {
    factor_set(mdl->factors[j], 0, -0.25 + 0.5 * rng_normal(R));
    factor_set(mdl->factors[j], 2,  0.45 + 0.5 * rng_normal(R));
  }

  /* optimize. */
  optim_t *opt = optim_alloc(optim_type_fg, mdl);
  opt->max_iters = 50;
  opt->l0 = 1.0;
  opt->dl = 0.1;
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
  rng_free(R);

  /* return success. */
  return 0;
}

