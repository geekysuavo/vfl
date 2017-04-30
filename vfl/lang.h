
/* ensure once-only inclusion. */
#ifndef __VFL_LANG_H__
#define __VFL_LANG_H__

/* include c headers. */
#include <stdio.h>
#include <stdlib.h>

/* include vfl headers. */
#include <vfl/lang/object.h>

/* function declarations, allocation (lang/): */

int vfl_exec_file (FILE *fh);

int vfl_exec_path (const char *fname);

int vfl_exec_string (const char *str);

#endif /* !__VFL_LANG_H__ */

