
# import the required modules.
import vfl

# load the input dataset.
dat = vfl.Data(file = 'co2.dat')

# shift the data locations for simpler inference.
for d in dat:
  d[0] -= 1995

# create a model without an explicit linear trend.
mdl = {}
mdl[False] = vfl.model.VFR(
  alpha0 = 100,
  beta0 = 1,
  nu = 1e-6,
  data = dat,
  factors = [vfl.factor.Cosine(mu = 0,    tau = 1e5),
             vfl.factor.Cosine(mu = 1e-3, tau = 1e4),
             vfl.factor.Cosine(mu = 1e-2, tau = 100),
             vfl.factor.Cosine(mu = 1e-1, tau = 100),
             vfl.factor.Cosine(mu = 5,    tau = 1),
             vfl.factor.Cosine(mu = 10,   tau = 1)]
)

# create a model with an explicit linear trend.
mdl[True] = vfl.model.VFR(
  alpha0 = 100,
  beta0 = 1,
  nu = 1e-6,
  data = dat,
  factors = [vfl.factor.Polynomial(order = 1),
             vfl.factor.Cosine(mu = 1e-3, tau = 1e4),
             vfl.factor.Cosine(mu = 1e-2, tau = 100),
             vfl.factor.Cosine(mu = 1e-1, tau = 100),
             vfl.factor.Cosine(mu = 5,    tau = 1),
             vfl.factor.Cosine(mu = 10,   tau = 1)]
)

# create an optimizer.
opt = vfl.optim.FullGradient(
  max_iters = 200,
  lipschitz_init = 1e-4
)

# build a grid for prediction.
S = {False: 'n', True: 'y'}
G = [[-25, 1e-2, 75]]

# treat both models.
for b in mdl:
  # optimize.
  opt.model = mdl[b]
  opt.execute()

  # build the output datasets.
  mean = vfl.Data(grid = G)
  var = vfl.Data(grid = G)

  # compute the model prediction.
  mdl[b].predict(mean = mean, var = var)

  # shift the predictions back to the original data locations.
  for d in mean: d[0] += 1995
  for d in var:  d[0] += 1995

  # write the prediction results.
  mean.write(file = 'mean-{}.out'.format(S[b]))
  var.write(file = 'var-{}.out'.format(S[b]))

