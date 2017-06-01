
# whether or not to use:
#  USE_ATLAS: atlas for blas/lapack.
#  USE_OPENCL: gpu-accelerated searches.
USE_ATLAS=y
USE_OPENCL=y

# executable filenames.
CC=gcc
RM=rm -f
WC=wc -l
LEX=flex
YACC=bison
GREP=grep -RHni --color

# compilation and linkage flags.
CFLAGS+= -fPIC -ggdb -O3 -std=gnu99 -I..
CFLAGS+= -Wall -Wextra -Wno-unused-parameter
LDFLAGS+= -shared
LIBS=-lm -ldl

# flex/bison generation flags.
LFLAGS=
YFLAGS=-d

# output library filename.
LIB=libvfl.so

# basenames of C utility source files.
C_UTIL=specfun vector matrix grid blas chol eigen

# basenames of C base type source files.
C_BASE=object list map int float string rng timer math std

# basenames of C, flex, and bison source files.
C_LANG=symbols ast eval
L_LANG=scanner
Y_LANG=parser

# basenames of C factor source files.
C_FACTOR=cosine decay impulse fixed-impulse polynomial product

# basenames of C model source files.
C_MODEL=vfc vfr tauvfr

# basenames of C optimizer source files.
C_OPTIM=fg mf

# extra flags for using atlas blas/lapack.
ifeq ($(USE_ATLAS),y)
CFLAGS+= -D__VFL_USE_ATLAS
LIBS+= -ltatlas
endif

# extra flags for using opencl.
ifeq ($(USE_OPENCL),y)
CFLAGS+= -D__VFL_USE_OPENCL
LIBS+= -lOpenCL
endif

# basenames of C source files in sub-directories.
C=  $(addprefix util/,$(C_UTIL))
C+= $(addprefix lang/,$(Y_LANG))
C+= $(addprefix lang/,$(L_LANG))
C+= $(addprefix lang/,$(C_LANG))
C+= $(addprefix base/,$(C_BASE))
C+= $(addprefix factor/,$(C_FACTOR)) factor-obj factor
C+= $(addprefix model/,$(C_MODEL)) model-obj model
C+= $(addprefix optim/,$(C_OPTIM)) optim-obj optim

# basenames of core C source files.
C+= data-alloc data-entries data-fileio data-sort data-obj
C+= datum-obj datum search-obj search vfl

# filenames of C sources.
SRC=$(addsuffix .c,$(C))

# filenames of compiled objects.
OBJ=$(addsuffix .o,$(C))

# filenames of generated sources.
GEN=  $(addsuffix .c,$(addprefix lang/,$(L_LANG)))
GEN+= $(addsuffix .c,$(addprefix lang/,$(Y_LANG)))
GEN+= $(addsuffix .h,$(addprefix lang/,$(Y_LANG)))

# installation directories.
DESTDIR=
PREFIX=$(DESTDIR)/usr/local
LIBDIR=$(PREFIX)/lib

# non-file targets and accepted filename extensions.
.PHONY: all clean again install lines fixme
.SUFFIXES: .c .l .y .o
.PRECIOUS: %.c %.l %.y %.o

# global, default make target.
all: $(LIB)

# library linkage target.
$(LIB): $(OBJ)
	@echo " LD   $@"
	@$(CC) $(LDFLAGS) $^ -o $@ $(LIBS)

# source compilation target.
.c.o:
	@echo " CC   $^"
	@$(CC) $(CFLAGS) -c $^ -o $@

# flex scanner generator target.
.l.c:
	@echo " LEX  $^"
	@$(LEX) $(LFLAGS) -o$@ $^

# bison parser generator target.
.y.c:
	@echo " YACC $^"
	@$(YACC) $(YFLAGS) -o $@ $^

# intermediate file cleanup target.
clean:
	@echo " CLEAN"
	@$(RM) $(LIB) $(OBJ) $(GEN)

# full recompilation target.
again: clean all

# installation target.
install: $(LIB)
	@echo " INSTALL $(LIB)"
	@install -d $(LIBDIR)
	@install $(LIB) $(LIBDIR)

# line-count reporting target.
lines: clean
	@echo " WC"
	@$(WC) $(shell find * -name '*.[cly]')

# fixme statement reporting target.
fixme:
	@echo " FIXME"
	@$(GREP) fixme $(shell find * -name '*.c') || echo " None found"

