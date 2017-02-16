
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
.PHONY: all clean again lines fixme dist

# global, default make target.
all:

# intermediate file cleanup target.
clean:
	@$(MAKE) -sC lib clean
	@$(MAKE) -sC tests/cosines clean
	@$(MAKE) -sC tests/sinc clean
	@$(MAKE) -sC tests/sinc-fixed clean

# full recompilation target.
again: clean all

# line-count reporting target.
lines:
	@echo " WC lib"
	@$(WC) $(shell find lib -name '*.c')
	@echo " WC vfl"
	@$(WC) $(shell find vfl -name '*.h')

# fixme statement reporting target.
fixme:
	@echo " FIXME lib"
	@$(GREP) fixme lib/*.c || echo " None found"
	@echo " FIXME vfl"
	@$(GREP) fixme vfl/*.h || echo " None found"

# tarball creation target.
dist: clean
	@echo " DIST $(DATE)"
	@$(RM) ../$(DIR)-$(DATE).tgz
	@$(CD) .. && $(TAR) $(DIR)-$(DATE).tgz $(DIR)/ && $(CD) $(DIR)

