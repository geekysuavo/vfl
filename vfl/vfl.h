
/* ensure once-only inclusion. */
#ifndef __VFL_VFL_H__
#define __VFL_VFL_H__

/* include core vfl headers. */
#include <vfl/data.h>
#include <vfl/model.h>
#include <vfl/optim.h>
#include <vfl/factor.h>

/* function declarations (vfl.c): */

int vfl_init (void);

int vfl_register_model_type (const model_type_t *type);

int vfl_register_optim_type (const optim_type_t *type);

int vfl_register_factor_type (const factor_type_t *type);

model_t *vfl_alloc_model (const char *name);

optim_t *vfl_alloc_optim (const char *name);

factor_t *vfl_alloc_factor (const char *name);

#endif /* !__VFL_VFL_H__ */

