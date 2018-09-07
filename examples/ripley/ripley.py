
# import the required modules.
from random import normalvariate
from math import sqrt
import vfl

# define the randomization parameters.
x0 = -0.25
y0 =  0.45
sigma = 0.5

# create a model.
mdl = vfl.model.VFC(
  nu = 1e-6,
  data = vfl.Data(file = 'ripley.dat'),
  factors = [vfl.factor.Impulse(dim = 0, mu = 0, tau = 10) *
             vfl.factor.Impulse(dim = 1, mu = 0, tau = 10)
             for i in range(10)]
)

# randomize the factor means.
for f in mdl:
  f[0].mu = normalvariate(x0, sigma)
  f[1].mu = normalvariate(y0, sigma)
  f.update()

# optimize.
opt = vfl.optim.FullGradient(model = mdl)
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

