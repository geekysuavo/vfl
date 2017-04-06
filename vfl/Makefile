
# basenames and filenames of headers in vfl
H_BASE=data factor model optim vfl
HDR_BASE=$(addsuffix .h,$(H_BASE))

# basenames and filenames of headers in vfl/util
H_UTIL=rng specfun vector matrix blas chol eigen
HDR_UTIL=$(addsuffix .h,$(addprefix util/,$(H_UTIL)))

# installation directories.
DESTDIR=
PREFIX=$(DESTDIR)/usr/local
INCDIR=$(PREFIX)/include/vfl

# non-file targets and accepted filename extensions.
.PHONY: all install
.SUFFIXES: .h

# global, default (nilpotent) make target.
all:

# installation target.
install:
	@echo " INSTALL vfl"
	@install -d $(INCDIR)
	@install -d $(INCDIR)/util
	@install $(HDR_BASE) $(INCDIR)
	@install $(HDR_UTIL) $(INCDIR)/util

