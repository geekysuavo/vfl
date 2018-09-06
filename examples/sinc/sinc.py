
# import the required modules.
from random import normalvariate
import vfl

# create a model.
mdl = vfl.model.VFR(
  alpha0 = 1000,
  beta0 = 2.5,
  nu = 1e-3,
  data = vfl.Data(file = 'sinc.dat'),
  factors = [vfl.factor.Impulse(mu = 0, tau = 0.01)
             for i in range(10)]
)

# randomize the factor means.
for f in mdl:
  f.mu = normalvariate(0, 2.5)

# create an optimizer.
opt = vfl.optim.FullGradient(
  model = mdl,
  lipschitz_init = 0.0001
)

# optimize.
opt.execute()

# build gridded datasets for prediction.
G = [[-10, 1e-3, 10]]
mean = vfl.Data(grid = G)
var = vfl.Data(grid = G)

# compute the model prediction.
mdl.predict(mean = mean, var = var)

# write the prediction results.
mean.write(file = 'mean.out')
var.write(file = 'var.out')

