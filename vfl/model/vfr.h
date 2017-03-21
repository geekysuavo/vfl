
/* ensure once-only inclusion. */
#ifndef __VFL_MODEL_VFR_H__
#define __VFL_MODEL_VFR_H__

/* function declarations (model/vfr.c): */

model_t *model_vfr (const double alpha0,
                    const double beta0,
                    const double nu);

MODEL_BOUND    (vfr);
MODEL_PREDICT  (vfr);
MODEL_INFER    (vfr);
MODEL_UPDATE   (vfr);
MODEL_GRADIENT (vfr);

#endif /* !__VFL_MODEL_VFR_H__ */

