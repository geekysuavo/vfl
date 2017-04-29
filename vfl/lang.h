
/* ensure once-only inclusion. */
#ifndef __VFL_LANG_H__
#define __VFL_LANG_H__

/* include c headers. */
#include <stdio.h>
#include <stdlib.h>

/* include vfl headers. */
#include <vfl/lang/object.h>

/* function declarations, allocation (lang/): */

/* FIXME: implement proper parser interaction layer. */
extern int yyparse (void);
#define vflang_parse() yyparse()

#endif /* !__VFL_LANG_H__ */

