
# figure filename.
PLT=ping.eps

# intermediate filenames.
DIR=../../tests/ping
DAT=ping.dat ping.mdl

# include the base figure makefile.
include ../figs.mk

# intermediate file rules:

ping.dat: $(DIR)/ping.dat
	@$(call mkdat, $^, $@)

ping.mdl: $(DIR)/mean.dat $(DIR)/var.dat
	@$(call mkmdl, $^, $@)

