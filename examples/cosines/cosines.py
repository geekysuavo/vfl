
# import the required modules.
from random import normalvariate
import vfl

# create a model.
mdl = vfl.model.TauVFR(
  tau = 1,
  nu = 1e-6,
  data = vfl.Data(file = 'cosines.dat'),
  factors = [vfl.factor.Cosine(tau = 1e-5),
             vfl.factor.Cosine(tau = 1e-5),
             vfl.factor.Cosine(tau = 1e-5),
             vfl.factor.Cosine(tau = 1e-5)]
)

# randomize the factor means.
for f in mdl:
  f.mu = normalvariate(0, 300)

# create an optimizer.
opt = vfl.optim.FullGradient(
  model = mdl,
  lipschitz_init = 0.001
)

# optimize.
opt.execute()

# build gridded datasets for prediction.
G = [[0, 1e-3, 0.5]]
mean = vfl.Data(grid = G)
var = vfl.Data(grid = G)

# compute the model prediction.
mdl.predict(mean = mean, var = var)

# write the prediction results.
mean.write(file = 'mean.out')
var.write(file = 'var.out')

