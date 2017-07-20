
# figure filename.
PLT=weights.eps

# input directories.
DIR_FIX=../../tests/ripley-fixed
DIR_VAR=../../tests/ripley

# intermediate filenames.
DAT=fix.dat var.dat

# include the base figure makefile.
include ../figs.mk

# intermediate file rules:

fix.dat: $(DIR_FIX)/ripley.out
	@$(call mksrt, $^, $@)

var.dat: $(DIR_VAR)/ripley.out
	@$(call mksrt, $^, $@)

