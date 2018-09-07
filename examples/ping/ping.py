
# import the required modules.
from random import normalvariate
from math import sqrt
import vfl

# create a model.
mdl = vfl.model.VFR(
  alpha0 = 100,
  beta0 = 100,
  nu = 1e-6,
  data = vfl.Data(file = 'ping.dat'),
  factors = vfl.factor.Decay(alpha = 200, beta = 1000) *
            vfl.factor.Cosine(mu = 0, tau = 0.1)
)

# randomize the frequency mean.
mdl.factors[0][1].mu = normalvariate(0, 1/sqrt(0.1))
mdl.factors[0].update()

# optimize.
opt = vfl.optim.FullGradient(model = mdl)
opt.execute()

# build gridded datasets for prediction.
G = [[0, 1e-3, 10]]
mean = vfl.Data(grid = G)
var = vfl.Data(grid = G)

# compute the model prediction.
mdl.predict(mean = mean, var = var)

# write the prediction results.
mean.write(file = 'mean.out')
var.write(file = 'var.out')

