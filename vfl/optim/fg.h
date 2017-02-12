
/* ensure once-only inclusion. */
#ifndef __VFL_OPTIM_FG_H__
#define __VFL_OPTIM_FG_H__

/* function declarations (optim-fg.c): */

optim_t *optim_fg (model_t *mdl);

int fg_iterate (optim_t *opt);

#endif /* !__VFL_OPTIM_FG_H__ */

