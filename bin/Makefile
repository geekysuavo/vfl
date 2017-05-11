
# whether or not to use:
#  USE_LIBUV: libuv for tcp communication.
USE_LIBUV=y

# executable filenames.
CC=gcc
RM=rm -f
WC=wc -l
GREP=grep -RHni --color

# compilation and linkage flags.
CFLAGS+= -ggdb -O3 -std=gnu99 -I.. -Wall -Wextra
LDFLAGS+= -L../lib

# required libraries.
LIBS=-lvfl -lreadline -lncurses

# output binary filename.
BIN=vflang

# basenames of C source files.
C=

# extra flags for using libuv.
ifeq ($(USE_LIBUV),y)
CFLAGS+= -D__VFL_USE_LIBUV
C+= comm-reply comm-client comm-server
LIBS+= -luv
endif

# filenames of C sources.
C+= $(BIN)
SRC=$(addsuffix .c,$(C))

# filenames of compiled objects.
OBJ=$(addsuffix .o,$(C))

# installation directories.
DESTDIR=
PREFIX=$(DESTDIR)/usr/local
BINDIR=$(PREFIX)/bin

# non-file targets and accepted filename extensions.
.PHONY: all clean again lines fixme
.SUFFIXES: .c .o
.PRECIOUS: %.c %.o

# global, default make target.
all: $(BIN)

# binary linkage make target.
$(BIN): $(OBJ)
	@echo " LD   $@"
	@$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ $(LIBS)

# source compilation target.
.c.o:
	@echo " CC   $^"
	@$(CC) $(CFLAGS) -c $^ -o $@

# intermediate file cleanup target.
clean:
	@echo " CLEAN"
	@$(RM) -r $(OBJ) $(BIN) $(BIN).dSYM

# full recompilation target.
again: clean all

# installation target.
install: $(BIN)
	@echo " INSTALL $(BIN)"
	@install -d $(BINDIR)
	@install $(BIN) $(BINDIR)

# line-count reporting target.
lines: clean
	@echo " WC"
	@$(WC) $(shell find * -name '*.c')

# fixme statement reporting target.
fixme:
	@echo " FIXME"
	@$(GREP) fixme $(shell find * -name '*.[ch]') || echo " None found"

