
# basenames and filenames of headers in vfl
H=data datum factor model optim search vfl
HDR=$(addsuffix .h,$(H))

# basenames and filenames of headers in vfl/base
H_BASE=object int float string list map rng math std
HDR_BASE=$(addsuffix .h,$(addprefix base/,$(H_BASE)))

# basenames and filenames of headers in vfl/lang
H_LANG=ast symbols
HDR_LANG=$(addsuffix .h,$(addprefix lang/,$(H_LANG)))

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
	@install -d $(INCDIR)/base
	@install -d $(INCDIR)/lang
	@install -d $(INCDIR)/util
	@install $(HDR) $(INCDIR)
	@install $(HDR_BASE) $(INCDIR)/base
	@install $(HDR_LANG) $(INCDIR)/lang
	@install $(HDR_UTIL) $(INCDIR)/util

