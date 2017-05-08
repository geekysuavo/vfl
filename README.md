
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

### Miscellaneous types

The VFL framework also implements the following base types:

 * **int**: signed long integers.
 * **float**: double-precision floats.
 * **string**: simple character strings.
 * **list**: ordered lists of objects.
 * **map**: associative arrays of objects, keyed by strings.

As well as the following types that prove useful for inference
and active learning:

 * **data**: datasets for organizing inputs and outputs.
 * **datum**: individual entries of dataset objects.
 * **rng**: pseudorandom number generator (LCG).
 * **search**: gaussian process posterior variance search.

## Programming

The VFL framework may be used in one of two ways:

 * [C99](http://en.wikipedia.org/wiki/C99) API
 * **vflang**

Using **vfl** through the C application programming interface enables
tight integration with any existing compiled programs or libraries,
but requires a strong knowledge of C programming principles.

Alternatively, **vfl** contains a lightweight interpreter that can
recognize a simple object-oriented language (_"vflang"_), which
enables direct access to the inference framework. Examples of
both C and **vflang** programs are provided in [tests](tests/).

## Installation

The **vfl** library and **vflang** interpreter are both written
in C99-compliant source code (with GNU extensions). Compiling
them requires **gcc**, **flex**, and **bison**.

Installation using the default options may be done as follows:

```bash
git clone git://github.com/geekysuavo/vfl.git
cd vfl
make
sudo make install
```

By default, **vfl** requires the
[ATLAS](http://math-atlas.sourceforge.net) library (with
[CLAPACK](http://netlib.org/clapack/) support compiled in)
for its linear algebra routines. This feature may be disabled
as follows:

```bash
sed -e 's,^\(USE_ATLAS\)=.*,\1=n,' -i lib/Makefile
make again
```

In addition, the **search** object defaults to using
[OpenCL](https://en.wikipedia.org/wiki/OpenCL), so
compilation will require suitable headers, libraries,
and drivers. Support for OpenCL may be disabled as follows:

```bash
sed -e 's,^\(USE_OPENCL\)=.*,\1=n,' -i lib/Makefile
make again
```

Finally, the default installation directory may be modified using
the usual suspects, _DESTDIR_ and _PREFIX_. By default, **vfl**
will install into the **/usr/local** prefix. For example:

```bash
sudo make PREFIX=/opt/vfl install
```

## Licensing

The **vfl** library is released under the
[MIT license](https://opensource.org/licenses/MIT). See the
[LICENSE.md](LICENSE.md) file for the complete license terms.

Enjoy!

*~ Brad.*

