
# basenames and filenames of headers in vfl
H=data datum factor model optim search vfl
HDR=$(addsuffix .h,$(H))

# basenames and filenames of headers in vfl/util
H_UTIL=blas chol eigen grid vector matrix specfun
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
	@install $(HDR) $(INCDIR)
	@install $(HDR_UTIL) $(INCDIR)/util

