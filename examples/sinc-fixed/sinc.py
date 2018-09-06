
# import the required modules.
import vfl

# create a model.
mdl = vfl.model.VFR(
  alpha0 = 1000,
  beta0 = 10,
  nu = 1e-3,
  data = vfl.Data(file = 'sinc.dat')
)

# add a fixed impulse factor at each data point.
mdl.factors = [vfl.factor.FixedImpulse(mu = d[0], tau = 0.001)
               for d in mdl.data]

# fix the factor precisions.
for f in mdl:
  f.tau = 1

# create an optimizer.
opt = vfl.optim.FullGradient(
  model = mdl,
  lipschitz_init = 0.001
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

