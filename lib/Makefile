
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
C_UTIL=rng specfun vector matrix blas chol eigen data

# basenames of C factor source files.
C_FACTOR=cosine impulse fixed-impulse polynomial

# basenames of C model source files.
C_MODEL=vfr

# basenames of C optimizer source files.
C_OPTIM=fg

# basenames of all C source files.
C=$(C_UTIL)
C+= factor $(addprefix factor/,$(C_FACTOR))
C+= model $(addprefix model/,$(C_MODEL))
C+= optim $(addprefix optim/,$(C_OPTIM))

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
	@$(WC) src/*.[ch]

# fixme statement reporting target.
fixme:
	@echo " FIXME"
	@$(GREP) fixme *.[ch] || echo " None found"

