
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

 * **Cosine**: sinusoids, inferred phase.
 * **Decay**: exponential decays.
 * **FixedImpulse**: delta functions, fixed location.
 * **Impulse**: delta functions, inferred location.
 * **Polynomial**: polynomials of fixed order.
 * **Product**: products of two or more factors.

### Models

The VFL framework supports three basic model types:

 * **VFC**: variational feature classification.
 * **VFR**: variational feature regression, inferred noise precision.
 * **TauVFR**: variational feature regression, fixed noise precision.

### Optimizers

At present two optimizers ship with VFL:

 * **FullGradient**: full-gradient optimization.
 * **MeanField**: mean-field optimization.

### Miscellaneous types

The VFL framework also implements the following types that prove
useful for inference and active learning:

 * **Data**: datasets for organizing inputs and outputs.
 * **Datum**: individual entries of dataset objects.
 * **Search**: gaussian process posterior variance search.

## Programming

The VFL framework is a Python C extension module. Example Python scripts
utilizing VFL for a selection of inference problems are provided within
[examples](examples/).

## Installation

The **vfl** module is written in C99-compliant source code
(with GNU extensions). Compiling it requires Python3.

Installation using the default options may be done as follows:

```bash
git clone git://github.com/geekysuavo/vfl.git
cd vfl
python3 setup.py build
python3 setup.py test
python3 setup.py install
```

By default, **vfl** does not require any external libraries. However,
it can optionally be compiled and linked against the
[ATLAS](http://math-atlas.sourceforge.net) library (with
[CLAPACK](http://netlib.org/clapack/) support compiled in)
for its linear algebra routines. This feature may be enabled
at build-time as follows:

```bash
python3 setup.py --with-atlas build
```

In addition, the **Search** object can be compiled and linked against
[OpenCL](https://en.wikipedia.org/wiki/OpenCL) to speed posterior
predictive variance evaluation. Support for OpenCL may be enabled
(again at build-time) as follows:

```bash
python3 setup.py --with-opencl build
```

## Licensing

The **vfl** library is released under the
[MIT license](https://opensource.org/licenses/MIT). See the
[LICENSE.md](LICENSE.md) file for the complete license terms.

Enjoy!

~ Brad.

