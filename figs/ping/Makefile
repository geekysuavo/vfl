
DIR=../../tests/ping
DAT_IN=ping.dat ping.p0.dat
DAT=$(DAT_IN) ping.eps

all: ping.eps

ping.eps: $(DAT_IN)
	@echo " PLT  $@"
	@gnuplot ping.gp

ping.dat: $(DIR)/ping.dat
	@echo " PROC $@"
	@../../scripts/dat.py $^ > $@

ping.p0.dat: $(DIR)/mean.dat $(DIR)/var.dat
	@echo " PROC $@"
	@../../scripts/stitch.py $^ ping

clean:
	@echo " CLEAN"
	@rm -f $(DAT)

again: clean all

