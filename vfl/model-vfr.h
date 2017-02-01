
/* ensure once-only inclusion. */
#ifndef __VFL_MODEL_VFR_H__
#define __VFL_MODEL_VFR_H__

/* function declarations (model-vfr.c): */

model_t *model_vfr (const double alpha0,
                    const double beta0,
                    const double nu);

double vfr_bound (const model_t *mdl);

int vfr_predict (const model_t *mdl, const vector_t *x,
                 double *mean, double *var);

int vfr_infer (model_t *mdl);

int vfr_update (model_t *mdl, const unsigned int j);

int vfr_gradient (const model_t *mdl, const unsigned int i,
                  const unsigned int j, vector_t *grad);

#endif /* !__VFL_MODEL_VFR_H__ */

