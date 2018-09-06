
# import the required modules.
import vfl

# create a model.
mdl = vfl.model.VFR(
  alpha0 = 10,
  beta0 = 40,
  nu = 1e-3,
  data = vfl.Data(file = 'multexp.dat'),
  factors = [vfl.factor.Decay(alpha = 10, beta =     1),
             vfl.factor.Decay(alpha = 10, beta =    10),
             vfl.factor.Decay(alpha = 10, beta =   100),
             vfl.factor.Decay(alpha = 10, beta =  1000),
             vfl.factor.Decay(alpha = 10, beta = 10000)]
)

# create an optimizer.
opt = vfl.optim.FullGradient(
  model = mdl,
  lipschitz_init = 0.001
)

# optimize.
opt.execute()

# build gridded datasets for prediction.
G = [[0, 0.1, 150]]
mean = vfl.Data(grid = G)
var = vfl.Data(grid = G)

# compute the model prediction.
mdl.predict(mean = mean, var = var)

# write the prediction results.
mean.write(file = 'mean.out')
var.write(file = 'var.out')

