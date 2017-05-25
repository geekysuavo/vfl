
# executable filenames.
CD=cd
WC=wc -l
RM=rm -f
TAR=tar czf
GREP=grep -RHni --color

# directory and date for tarball creation.
DIR=vfl
DATE=$(shell date +%Y%m%d)

# non-file targets.
.PHONY: all clean again install lines fixme dist

# global, default make target.
all:
	@$(MAKE) -sC lib
	@$(MAKE) -sC bin

# intermediate file cleanup target.
clean: clean-tests clean-figs
	@$(MAKE) -sC lib clean
	@$(MAKE) -sC bin clean

# test result cleanup target.
clean-tests:
	@$(MAKE) -sC tests/carbon-dioxide clean
	@$(MAKE) -sC tests/cosines clean
	@$(MAKE) -sC tests/gauss clean
	@$(MAKE) -sC tests/methane clean
	@$(MAKE) -sC tests/multexp clean
	@$(MAKE) -sC tests/ping clean
	@$(MAKE) -sC tests/poly clean
	@$(MAKE) -sC tests/ripley clean
	@$(MAKE) -sC tests/ripley-fixed clean
	@$(MAKE) -sC tests/sinc clean
	@$(MAKE) -sC tests/sinc-fixed clean

# figure cleanup target.
clean-figs:
	@$(MAKE) -sC figs/decays clean
	@$(MAKE) -sC figs/environ clean
	@$(MAKE) -sC figs/ping clean
	@$(MAKE) -sC figs/ripley clean

# full recompilation target.
again: clean all

# installation target.
install:
	@$(MAKE) -sC lib install
	@$(MAKE) -sC bin install
	@$(MAKE) -sC vfl install

# line-count reporting target.
lines: clean
	@echo " WC bin"
	@$(WC) $(shell find bin -name '*.c')
	@echo " WC lib"
	@$(WC) $(shell find lib -name '*.[cly]')
	@echo " WC vfl"
	@$(WC) $(shell find vfl -name '*.h')

# fixme statement reporting target.
fixme:
	@echo " FIXME bin"
	@$(GREP) fixme $(shell find bin -name '*.c') || echo " None found"
	@echo " FIXME lib"
	@$(GREP) fixme $(shell find lib -name '*.[cly]') || echo " None found"
	@echo " FIXME vfl"
	@$(GREP) fixme $(shell find vfl -name '*.h') || echo " None found"

# tarball creation target.
dist: clean
	@echo " DIST $(DATE)"
	@$(RM) ../$(DIR)-$(DATE).tgz
	@$(CD) .. && $(TAR) $(DIR)-$(DATE).tgz $(DIR)/ && $(CD) $(DIR)

