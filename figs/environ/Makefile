
# figure filename.
PLT=environ.eps

# input directories.
DIR_CO2=../../tests/carbon-dioxide
DIR_CH4=../../tests/methane

# intermediate filenames.
DAT=co2.dat ch4.dat co2-a.mdl co2-b.mdl ch4-a.mdl ch4-b.mdl

# include the base figure makefile.
include ../figs.mk

# intermediate file rules:

co2.dat: $(DIR_CO2)/co2.dat
	@$(call mkdat, $^, $@)

co2-a.mdl: $(DIR_CO2)/mean-a.dat $(DIR_CO2)/var-a.dat
	@$(call mkmdl, $^, $@)

co2-b.mdl: $(DIR_CO2)/mean-b.dat $(DIR_CO2)/var-b.dat
	@$(call mkmdl, $^, $@)

ch4.dat: $(DIR_CH4)/ch4.dat
	@$(call mkdat, $^, $@)

ch4-a.mdl: $(DIR_CH4)/mean-a.dat $(DIR_CH4)/var-a.dat
	@$(call mkmdl, $^, $@)

ch4-b.mdl: $(DIR_CH4)/mean-b.dat $(DIR_CH4)/var-b.dat
	@$(call mkmdl, $^, $@)

