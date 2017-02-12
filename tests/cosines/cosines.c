
/* include the required headers. */
#include <vfl/optim.h>
#include <vfl/rng.h>

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
  data_t *dat = data_alloc();
  data_fread(dat, "cosines.dat");

  /* set up a regression model. */
  model_t *mdl = model_vfr(1000.0, 1000.0, 1.0e-6);
  model_set_data(mdl, dat);

  /* add factors to the model. */
  const unsigned int M = 4;
  for (unsigned int j = 0; j < M; j++) {
    factor_t *f = factor_cosine(0.0, 1.0e-5);
    model_add_factor(mdl, f);
  }

  /* randomly initialize the factor frequency means. */
  for (unsigned int j = 0; j < mdl->M; j++)
    factor_set(mdl->factors[j], 0, 300.0 * rng_normal(R));

  /* initialize the optimizer. */
  optim_t *opt = optim_fg(mdl);
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

  /* FIXME: move this to gridded evaluations. */
  vector_t *x = vector_alloc(dat->D);
  for (double xd = 0.0; xd <= 0.5; xd += 0.001) {
    double mean, var;
    vector_set(x, 0, xd);
    model_predict(mdl, x, &mean, &var);
    printf("%le %le %le\n", xd, mean, var);
  }

  /* free the structures. */
  vector_free(x);
  optim_free(opt);
  model_free(mdl);
  data_free(dat);
  rng_free(R);

  /* return success. */
  return 0;
}

