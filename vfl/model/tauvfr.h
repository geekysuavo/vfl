
/* ensure once-only inclusion. */
#ifndef __VFL_MODEL_TAUVFR_H__
#define __VFL_MODEL_TAUVFR_H__

/* function declarations (model/tauvfr.c): */

model_t *model_tauvfr (const double tau, const double nu);

double tauvfr_bound (const model_t *mdl);

int tauvfr_predict (const model_t *mdl, const vector_t *x,
                    double *mean, double *var);

int tauvfr_infer (model_t *mdl);

int tauvfr_update (model_t *mdl, const unsigned int j);

int tauvfr_gradient (const model_t *mdl, const unsigned int i,
                     const unsigned int j, vector_t *grad);

#endif /* !__VFL_MODEL_TAUVFR_H__ */

