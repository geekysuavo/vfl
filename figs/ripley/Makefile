
# figure filename.
PLT=ripley.eps

# input directories.
DIR_FIX=../../tests/ripley-fixed
DIR_VAR=../../tests/ripley

# intermediate filenames.
DAT=  fix.y0.dat fix.y1.dat fix.cut fix.lin
DAT+= var.y0.dat var.y1.dat var.cut var.lin

# include the base figure makefile.
include ../figs.mk

# intermediate file rules:

fix.y0.dat: $(DIR_FIX)/ripley.dat
	@$(call mkneg, $^, $@)

fix.y1.dat: $(DIR_FIX)/ripley.dat
	@$(call mkpos, $^, $@)

fix.cut: $(DIR_FIX)/mean.dat
	@$(call mkcntr, $^, $@, '[0 0.5 1]')

fix.lin: $(DIR_FIX)/mean.dat
	@$(call mkcntr, $^, $@, '[0 0.4 0.45 0.55 0.6 1]')

var.y0.dat: $(DIR_VAR)/ripley.dat
	@$(call mkneg, $^, $@)

var.y1.dat: $(DIR_VAR)/ripley.dat
	@$(call mkpos, $^, $@)

var.cut: $(DIR_VAR)/mean.dat
	@$(call mkcntr, $^, $@, '[0 0.5 1]')

var.lin: $(DIR_VAR)/mean.dat
	@$(call mkcntr, $^, $@, '[0 0.4 0.45 0.55 0.6 1]')

