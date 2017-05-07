
/* ensure once-only inclusion. */
#ifndef __VFL_VFL_H__
#define __VFL_VFL_H__

/* include vfl base object headers. */
#include <vfl/base/object.h>
#include <vfl/base/list.h>
#include <vfl/base/map.h>
#include <vfl/base/int.h>
#include <vfl/base/float.h>
#include <vfl/base/string.h>
#include <vfl/base/rng.h>
#include <vfl/base/std.h>

/* include vfl inference object headers. */
#include <vfl/datum.h>
#include <vfl/data.h>
#include <vfl/model.h>
#include <vfl/optim.h>
#include <vfl/factor.h>
#include <vfl/search.h>

/* include vfl utility headers. */
#include <vfl/util/vector.h>
#include <vfl/util/matrix.h>

/* function declarations (vfl.c): */

int vfl_init (void);

void vfl_cleanup (void);

int vfl_register_type (const object_type_t *type);

object_type_t *vfl_lookup_type (const char *name);

int vfl_exec_file (FILE *fh);

int vfl_exec_path (const char *fname);

int vfl_exec_string (const char *str);

int vfl_import (const char *modname);

#endif /* !__VFL_VFL_H__ */

