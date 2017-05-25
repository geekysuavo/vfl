
# figure filename.
PLT=decays.eps

# input directories.
DIR_PING=../../tests/ping
DIR_MEXP=../../tests/multexp

# intermediate filenames.
DAT=  ping.dat multexp.dat
DAT+= ping.mdl multexp.mdl

# include the base figure makefile.
include ../figs.mk

# intermediate file rules:

ping.dat: $(DIR_PING)/ping.dat
	@$(call mkdat, $^, $@)

ping.mdl: $(DIR_PING)/mean.dat $(DIR_PING)/var.dat
	@$(call mkmdl, $^, $@)

multexp.dat: $(DIR_MEXP)/multexp.dat
	@$(call mkdat, $^, $@)

multexp.mdl: $(DIR_MEXP)/mean.dat $(DIR_MEXP)/var.dat
	@$(call mkmdl, $^, $@)

