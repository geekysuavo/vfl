
# executable filenames.
RM=rm -f
GP=gnuplot

# script filenames.
PYDIR=../../scripts
PY_CLS=$(PYDIR)/cls.py
PY_DAT=$(PYDIR)/dat.py
PY_STITCH=$(PYDIR)/stitch.py
OCT_CNTR=octave --no-gui $(PYDIR)/cntr.m

# function for creating data files.
define mkdat
echo " DATA$(2)"
$(PY_DAT) $(1) > $(2)
endef

# function for creating class files (y=0).
define mkneg
echo " CLS $(2)"
$(PY_CLS) 0 $(1) > $(2)
endef

# function for creating class files (y=1).
define mkpos
echo " CLS $(2)"
$(PY_CLS) 1 $(1) > $(2)
endef

# function for creating model files.
define mkmdl
echo " MDL $(2)"
$(PY_STITCH) $(word 1, $(1)) $(word 2, $(1)) mdl
mv mdl.p0.dat $(2)
endef

# function for creating contour files.
define mkcntr
echo " CNTR$(2)"
$(OCT_CNTR) $(1) $(3) > $(2)
endef

# global, default make target.
all: $(PLT)

# figure generation target.
$(PLT): $(DAT)
	@echo " PLT  $@"
	@$(GP) $(PLT:.eps=.gp)

# intermediate file cleanup target.
clean:
	@echo " CLEAN"
	@$(RM) $(DAT) $(PLT)

# full recompilation target.
again: clean all

