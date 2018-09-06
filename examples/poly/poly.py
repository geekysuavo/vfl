
# import the required modules.
import vfl

# create a model.
mdl = vfl.model.VFR(
  alpha0 = 1000,
  beta0 = 2.5,
  nu = 1e-3,
  data = vfl.Data(file = '../sinc/sinc.dat'),
  factors = vfl.factor.Polynomial(order = 10)
)

# infer the weight parameters.
mdl.infer()

# build gridded datasets for prediction.
G = [[-10, 1e-3, 10]]
mean = vfl.Data(grid = G)
var = vfl.Data(grid = G)

# compute the model prediction.
mdl.predict(mean = mean, var = var)

# write the prediction results.
mean.write(file = 'mean.out')
var.write(file = 'var.out')

