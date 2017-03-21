
/* ensure once-only inclusion. */
#ifndef __VFL_MODEL_TAUVFR_H__
#define __VFL_MODEL_TAUVFR_H__

/* function declarations (model/tauvfr.c): */

model_t *model_tauvfr (const double tau, const double nu);

MODEL_BOUND    (tauvfr);
MODEL_PREDICT  (tauvfr);
MODEL_INFER    (tauvfr);
MODEL_UPDATE   (tauvfr);
MODEL_GRADIENT (tauvfr);

#endif /* !__VFL_MODEL_TAUVFR_H__ */

