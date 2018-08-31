
# import the required functionality.
from distutils.core import setup, Extension
import os

# walk the 'src' directory to find all source files.
src = [os.path.join(d, f) for d, base, files in os.walk('src')
       for f in files if f.endswith('.c')]

# define the vfl extension module.
vfl_extension = Extension(
  name = 'vfl',
  include_dirs = ['.'],
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

  # project package and extension module list.
  ext_modules = [vfl_extension]
)

