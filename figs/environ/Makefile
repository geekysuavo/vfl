
DIR_CO2=../../tests/carbon-dioxide
DIR_CH4=../../tests/methane

DAT_CO2=co2.dat co2-a.dat co2-b.dat
DAT_CH4=ch4.dat ch4-a.dat ch4-b.dat
DAT=$(DAT_CO2) $(DAT_CH4) environ.eps

all: environ.eps

environ.eps: $(DAT_CO2) $(DAT_CH4)
	@echo " PLT  $@"
	@gnuplot environ.gp

co2.dat: $(DIR_CO2)/co2.dat
	@echo " PROC $@"
	@../../scripts/dat.py $^ > $@

ch4.dat: $(DIR_CH4)/ch4.dat
	@echo " PROC $@"
	@../../scripts/dat.py $^ > $@

co2-a.dat: $(DIR_CO2)/mean-a.dat $(DIR_CO2)/var-a.dat
	@echo " PROC $@"
	@../../scripts/stitch.py $^ co2
	@mv co2.p0.dat $@

co2-b.dat: $(DIR_CO2)/mean-b.dat $(DIR_CO2)/var-b.dat
	@echo " PROC $@"
	@../../scripts/stitch.py $^ co2
	@mv co2.p0.dat $@

ch4-a.dat: $(DIR_CH4)/mean-a.dat $(DIR_CH4)/var-a.dat
	@echo " PROC $@"
	@../../scripts/stitch.py $^ ch4
	@mv ch4.p0.dat $@

ch4-b.dat: $(DIR_CH4)/mean-b.dat $(DIR_CH4)/var-b.dat
	@echo " PROC $@"
	@../../scripts/stitch.py $^ ch4
	@mv ch4.p0.dat $@

clean:
	@echo " CLEAN"
	@rm -f $(DAT)

again: clean all

