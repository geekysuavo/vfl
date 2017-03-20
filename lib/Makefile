
# executable filenames.
CC=gcc
RM=rm -f
WC=wc -l
GREP=grep -RHni --color

# compilation and linkage flags.
CFLAGS=-fPIC -ggdb -O3 -std=gnu99 -I.. -Wall -Wextra -Wno-unused-parameter
LDFLAGS=-shared
LIBS=-lm

# output library filename.
LIB=libvfl.so

# basenames of C utility source files.
C_UTIL=rng specfun vector matrix blas chol eigen

# basenames of C factor source files.
C_FACTOR=cosine decay impulse fixed-impulse polynomial product

# basenames of C model source files.
C_MODEL=vfc vfr tauvfr

# basenames of C optimizer source files.
C_OPTIM=fg

# basenames of all C source files.
C=  data-alloc data-entries data-fileio factor model optim
C+= $(addprefix factor/,$(C_FACTOR))
C+= $(addprefix model/,$(C_MODEL))
C+= $(addprefix optim/,$(C_OPTIM))
C+= $(addprefix util/,$(C_UTIL))

# filenames of C sources.
SRC=$(addsuffix .c,$(C))

# filenames of compiled objects.
OBJ=$(addsuffix .o,$(C))

# non-file targets and accepted filename extensions.
.PHONY: all clean again lines fixme
.SUFFIXES: .c .o

# global, default make target.
all: $(LIB)

# library linkage make target.
$(LIB): $(OBJ)
	@echo " LD $@"
	@$(CC) $(LDFLAGS) $^ -o $@ $(LIBS)

# source compilation target.
.c.o:
	@echo " CC $^"
	@$(CC) $(CFLAGS) -c $^ -o $@

# intermediate file cleanup target.
clean:
	@echo " CLEAN"
	@$(RM) $(LIB) $(OBJ)

# full recompilation target.
again: clean all

# line-count reporting target.
lines:
	@echo " WC"
	@$(WC) $(shell find * -name '*.c')

# fixme statement reporting target.
fixme:
	@echo " FIXME"
	@$(GREP) fixme $(shell find * -name '*.c') || echo " None found"

