
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
  data_fread(dat, "ripley.dat");

  /* set up a classification model. */
  model_t *mdl = model_vfc(1.0e-6);
  model_set_data(mdl, dat);

  /* add factors to the model. */
  for (unsigned int i = 0; i < dat->N; i++) {
    /* get the factor location. */
    const double x = matrix_get(dat->X, i, 0);
    const double y = matrix_get(dat->X, i, 1);

    /* create the independent factors along each dimension. */
    factor_t *fx = factor_fixed_impulse(x, 0.1);
    factor_t *fy = factor_fixed_impulse(y, 0.1);

    /* create the combined factor. */
    factor_t *f = factor_product(2, 0, fx, 1, fy);
    model_add_factor(mdl, f);

    /* initialize the factor parameters. */
    factor_set(mdl->factors[mdl->M - 1], 0, 10.0);
    factor_set(mdl->factors[mdl->M - 1], 1, 10.0);
  }

  /* set up an optimizer. */
  optim_t *opt = optim_fg(mdl);
  opt->l0 = 0.001;
  opt->dl = 0.1;
  unsigned int iter;
  double bound, bprev;
  bound = -1.0e99;

  /* perform a few optimization iterations. */
  for (iter = 0; iter <= 50; iter++) {
    bprev = bound;
    bound = model_bound(mdl);
    if (bound < bprev) break;

    fprintf(stderr, "%u %le\n", iter, bound);
    fflush(stderr);

    if (!opt->iterate(opt)) break;
  }

  /* allocate datasets for prediction. */
  double grid_values[] = { -1.5, 0.01, 1.0,
                           -0.3, 0.01, 1.2 };
  matrix_view_t grid = matrix_view_array(grid_values, 2, 3);
  data_t *mean = data_alloc_from_grid(&grid);
  data_t *var = data_alloc_from_grid(&grid);

  /* compute the prediction. */
  model_predict_all(mdl, mean, var);
  data_fwrite(mean, "mean.dat");
  data_fwrite(var, "var.dat");

  /* free the structures. */
  model_free(mdl);
  data_free(mean);
  data_free(var);
  data_free(dat);

  /* return success. */
  return 0;
}

