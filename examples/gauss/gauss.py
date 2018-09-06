
# import the required modules.
import vfl

# create a model.
mdl = vfl.model.VFR(
  alpha0 = 100,
  beta0 = 100,
  nu = 1e-2,
  data = vfl.Data(file = 'gauss.dat'),
  factors = [vfl.factor.Decay(alpha = 10, beta = 1000),
             vfl.factor.Impulse(mu = 60, tau = 0.01),
             vfl.factor.Impulse(mu = 180, tau = 0.01)]
)

# optimize.
opt = vfl.optim.FullGradient(model = mdl)
opt.execute()

# build gridded datasets for prediction.
G = [[0, 1, 300]]
mean = vfl.Data(grid = G)
var = vfl.Data(grid = G)

# compute the model prediction.
mdl.predict(mean = mean, var = var)

# write the prediction results.
mean.write(file = 'mean.out')
var.write(file = 'var.out')

