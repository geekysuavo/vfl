
/* ensure once-only inclusion. */
#ifndef __VFL_MODEL_VFC_H__
#define __VFL_MODEL_VFC_H__

/* function declarations (model/vfc.c): */

model_t *model_vfc (const double nu);

MODEL_BOUND    (vfc);
MODEL_PREDICT  (vfc);
MODEL_INFER    (vfc);
MODEL_UPDATE   (vfc);
MODEL_GRADIENT (vfc);

#endif /* !__VFL_MODEL_VFC_H__ */

