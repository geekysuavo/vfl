
# whether or not to use atlas blas/lapack.
USE_ATLAS=y

# executable filenames.
CC=gcc
RM=rm -f
WC=wc -l
GREP=grep -RHni --color

# compilation and linkage flags.
CFLAGS+= -fPIC -ggdb -O3 -std=gnu99 -I..
CFLAGS+= -Wall -Wextra -Wno-unused-parameter
LDFLAGS+= -shared
LIBS=-lm

# extra flags for using atlas blas/lapack.
ifeq ($(USE_ATLAS),y)
ATLAS=/usr/local/atlas
CFLAGS+= -D__VFL_USE_ATLAS -I$(ATLAS)/include
LDFLAGS+= -L$(ATLAS)/lib
LIBS+= -ltatlas
endif

# output library filename.
LIB=libvfl.so

# basenames of C utility source files.
C_UTIL=rng specfun vector matrix blas chol eigen

# basenames of C factor source files.
C_FACTOR=cosine decay impulse fixed-impulse polynomial product

# basenames of C model source files.
C_MODEL=vfc vfr tauvfr

# basenames of C optimizer source files.
C_OPTIM=fg mf

# basenames of all C source files.
C=  vfl data-alloc data-entries data-fileio factor model optim
C+= $(addprefix factor/,$(C_FACTOR))
C+= $(addprefix model/,$(C_MODEL))
C+= $(addprefix optim/,$(C_OPTIM))
C+= $(addprefix util/,$(C_UTIL))

# filenames of C sources.
SRC=$(addsuffix .c,$(C))

# filenames of compiled objects.
OBJ=$(addsuffix .o,$(C))

# installation directories.
DESTDIR=
PREFIX=$(DESTDIR)/usr/local
LIBDIR=$(PREFIX)/lib

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

# installation target.
install: $(LIB)
	@echo " INSTALL $(LIB)"
	@install -d $(LIBDIR)
	@install $(LIB) $(LIBDIR)

# line-count reporting target.
lines:
	@echo " WC"
	@$(WC) $(shell find * -name '*.c')

# fixme statement reporting target.
fixme:
	@echo " FIXME"
	@$(GREP) fixme $(shell find * -name '*.c') || echo " None found"

