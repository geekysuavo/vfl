
CNTR=octave --no-gui ../../scripts/cntr.m

DAT_FIX=fix.cut fix.lin fix.y0.dat fix.y1.dat
DAT_VAR=var.cut var.lin var.y0.dat var.y1.dat
DAT=$(DAT_FIX) $(DAT_VAR) ripley.eps

all: ripley.eps

ripley.eps: $(DAT_FIX) $(DAT_VAR)
	@echo " PLT  $@"
	@gnuplot ripley.gp

fix.cut: ../../tests/ripley-fixed/mean.dat
	@echo " PROC $@"
	@$(CNTR) $^ '[0, 0.5, 1]' > $@

fix.lin: ../../tests/ripley-fixed/mean.dat
	@echo " PROC $@"
	@$(CNTR) $^ '[0, 0.4, 0.45, 0.55, 0.6, 1]' > $@

fix.y0.dat: ../../tests/ripley-fixed/ripley.dat
	@echo " PROC $@"
	@../../scripts/cls.py 0 $^ > $@

fix.y1.dat: ../../tests/ripley-fixed/ripley.dat
	@echo " PROC $@"
	@../../scripts/cls.py 1 $^ > $@

var.cut: ../../tests/ripley/mean.dat
	@echo " PROC $@"
	@$(CNTR) $^ '[0, 0.5, 1]' > $@

var.lin: ../../tests/ripley/mean.dat
	@echo " PROC $@"
	@$(CNTR) $^ '[0, 0.4, 0.45, 0.55, 0.6, 1]' > $@

var.y0.dat: ../../tests/ripley/ripley.dat
	@echo " PROC $@"
	@../../scripts/cls.py 0 $^ > $@

var.y1.dat: ../../tests/ripley/ripley.dat
	@echo " PROC $@"
	@../../scripts/cls.py 1 $^ > $@

clean:
	@echo " CLEAN"
	@rm -f $(DAT)

again: clean all

