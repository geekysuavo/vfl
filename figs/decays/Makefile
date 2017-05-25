
DIR_PING=../../tests/ping
DIR_MEXP=../../tests/multexp

DAT_PING=ping.dat ping.p0.dat
DAT_MEXP=multexp.dat multexp.p0.dat
DAT=$(DAT_MEXP) $(DAT_PING) decays.eps

all: decays.eps

decays.eps: $(DAT_PING) $(DAT_MEXP)
	@echo " PLT  $@"
	@gnuplot decays.gp

ping.dat: $(DIR_PING)/ping.dat
	@echo " PROC $@"
	@../../scripts/dat.py $^ > $@

ping.p0.dat: $(DIR_PING)/mean.dat $(DIR_PING)/var.dat
	@echo " PROC $@"
	@../../scripts/stitch.py $^ ping

multexp.dat: $(DIR_MEXP)/multexp.dat
	@echo " PROC $@"
	@../../scripts/dat.py $^ > $@

multexp.p0.dat: $(DIR_MEXP)/mean.dat $(DIR_MEXP)/var.dat
	@echo " PROC $@"
	@../../scripts/stitch.py $^ multexp

clean:
	@echo " CLEAN"
	@rm -f $(DAT)

again: clean all

