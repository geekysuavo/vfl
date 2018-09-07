
# import the required modules.
from math import sqrt
import vfl

# create a model.
mdl = vfl.model.VFC(
  nu = 1e-6,
  data = vfl.Data(file = 'ripley.dat'),
)

# add a fixed impulse factor at every tenth data point.
mdl.factors = [
  vfl.factor.Impulse(dim = 0, mu = mdl.data[i][0], tau = 10) *
  vfl.factor.Impulse(dim = 1, mu = mdl.data[i][1], tau = 10)
  for i in range(0, len(mdl.data), 10)]

# initialize the factor precisions.
for f in mdl:
  f[0].tau = 10
  f[1].tau = 10
  f.update()

# create an optimizer.
opt = vfl.optim.FullGradient(
  model = mdl,
  lipschitz_init = 0.001
)

# optimize.
opt.execute()

# build gridded datasets for prediction.
G = [[-1.5, 0.01, 1.0],
     [-0.3, 0.02, 1.2]]
mean = vfl.Data(grid = G)
var = vfl.Data(grid = G)

# compute the model prediction.
mdl.predict(mean = mean, var = var)

# write the prediction results.
mean.write(file = 'mean.out')
var.write(file = 'var.out')

# output a final set of statistics.
for j in range(len(mdl)):
  vals = (mdl.wbar[j],
          sqrt(mdl.Sigma[j][j]),
          1 / sqrt(mdl.factors[j][0].tau),
          1 / sqrt(mdl.factors[j][1].tau))
  print(('{} ' * 4).format(*vals))

