
/* ensure once-only inclusion. */
#ifndef __VFL_MODEL_VFC_H__
#define __VFL_MODEL_VFC_H__

/* function declarations (model/vfc.c): */

model_t *model_vfc (const double nu);

double vfc_bound (const model_t *mdl);

int vfc_predict (const model_t *mdl, const vector_t *x,
                 double *mean, double *var);

int vfc_infer (model_t *mdl);

int vfc_update (model_t *mdl, const unsigned int j);

int vfc_gradient (const model_t *mdl, const unsigned int i,
                  const unsigned int j, vector_t *grad);

#endif /* !__VFL_MODEL_VFC_H__ */

