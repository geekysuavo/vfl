
# Variational Feature Learning

A framework for approximate Bayesian inference in a large class of
regression and classification models. The details of VFR/VFC are
in preparation for submission in:

> Worley, B., Nilges, M., Malliavin, T. E., _Variational Features for
> Regression and Classification_, Journal of Machine Learning Research,
> 2017.

## Introduction

The core of a VFL model is a linear combination of feature functions,
which take a multidimensional input and return one or more outputs.
In VFL models, each feature function is called a _factor_, as the
joint distribution _is assumed to factorize_ with respect to the
priors and approximate posteriors placed over the feature
parameters. Furthermore, the prior and posterior distributions
over parameters of a given factor are constrained to be the
same type.

In general, VFL aims for modularity, flexibility, and extensibility.
Users can introduce new factors, models, and optimizers in order to
achieve radically different behavior.

### Factors

The current VFL framework supports the following built-in factors:

 * **cosine**: sinusoids, inferred phase.
 * **decay**: exponential decays.
 * **fixed-impulse**: delta functions, fixed location.
 * **impulse**: delta functions, inferred location.
 * **polynomial**: polynomials of fixed order.
 * **product**: products of two or more factors.

### Models

The VFL framework supports three basic model types:

 * **vfc**: variational feature classification.
 * **vfr**: variational feature regression, inferred noise precision.
 * **tauvfr**: variational feature regression, fixed noise precision.

### Optimizers

At present two optimizers ship with VFL:

 * **fg**: full-gradient optimization.
 * **mf**: mean-field optimization.

## Installation

FIXME.

## Licensing

The **vfl** library is released under the
[MIT license](https://opensource.org/licenses/MIT). See the
[LICENSE.md](LICENSE.md) file for the complete license terms.

Enjoy!

*~ Brad.*

