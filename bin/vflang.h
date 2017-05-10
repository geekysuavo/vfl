
/* include c library headers. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* include posix/gnu headers. */
#include <unistd.h>
#include <getopt.h>

/* include readline headers. */
#include <readline/readline.h>
#include <readline/history.h>

/* include vfl headers. */
#include <vfl/vfl.h>

/* include and define libuv-related objects. */
#ifdef __VFL_USE_LIBUV
# include "comm.h"
#endif

