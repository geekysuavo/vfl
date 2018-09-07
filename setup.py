
# import the required functionality.
from setuptools import setup, Extension
import unittest
import os, sys

# initialize the include directories list.
inc = ['.']

# walk the 'src' directory to find all source files.
src = [os.path.join(d, f) for d, base, files in os.walk('src')
       for f in files if f.endswith('.c')]

# initialize the extra compile arguments.
cflags = ['-std=c99', '-O3', '-Wall']

# initialize the libraries to link against.
libs = []

# initialize the macro definitions.
defs = []

# check if the user specified the use of ATLAS.
if '--with-atlas' in sys.argv:
  defs.append(('__VFL_USE_ATLAS', None))
  libs.append('tatlas')
  sys.argv.remove('--with-atlas')

# check if the user specified the use of OpenCL.
if '--with-opencl' in sys.argv:
  defs.append(('__VFL_USE_OPENCL', None))
  libs.append('OpenCL')
  sys.argv.remove('--with-opencl')

# define the vfl extension module.
vfl_extension = Extension(
  name = 'vfl',
  extra_compile_args = cflags,
  define_macros = defs,
  include_dirs = inc,
  libraries = libs,
  sources = src
)

# run the setup function.
setup(
  # package name and version.
  name = 'vfl',
  version = '0.0.1',

  # package descriptions.
  description = 'Variational Feature Learning',
  long_description = '''
''',

  # project uniform resource locator.
  url = 'http://github.com/geekysuavo/vfl',

  # project author.
  author = 'Bradley Worley',
  author_email = 'geekysuavo@gmail.com',

  # project license.
  license = 'MIT',

  # project classification.
  classifiers = [
    'Development Status :: 1 - Planning',
    'Environment :: Console',

    'Intended Audience :: Developers',
    'Intended Audience :: Science/Research',

    'Topic :: Scientific/Engineering :: Mathematics',

    'License :: OSI Approved :: MIT License',

    'Programming Language :: Python :: 3',
    'Programming Language :: Python :: 3.3',
    'Programming Language :: Python :: 3.5',
    'Programming Language :: Python :: 3.6'
  ],

  # project test suite.
  test_suite = 'tests',

  # project package and extension module list.
  ext_modules = [vfl_extension]
)

