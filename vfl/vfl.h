
/* ensure once-only inclusion. */
#ifndef __VFL_VFL_H__
#define __VFL_VFL_H__

/* include vfl language headers. */
#include <vfl/lang/object.h>
#include <vfl/lang.h>

/* include vfl core headers. */
#include <vfl/data.h>
#include <vfl/model.h>
#include <vfl/optim.h>
#include <vfl/factor.h>

/* include vfl utility headers. */
#include <vfl/util/list.h>
#include <vfl/util/int.h>
#include <vfl/util/float.h>
#include <vfl/util/string.h>
#include <vfl/util/search.h>
#include <vfl/util/rng.h>

/* include non-object vfl utility headers. */
#include <vfl/util/vector.h>
#include <vfl/util/matrix.h>

/* function declarations (vfl.c): */

int vfl_init (void);

int vfl_register_type (const object_type_t *type);

object_type_t *vfl_lookup_type (const char *name);

#endif /* !__VFL_VFL_H__ */

