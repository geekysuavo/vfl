
/* ensure once-only inclusion. */
#ifndef __VFL_LIST_H__
#define __VFL_LIST_H__

/* include vfl headers. */
#include <vfl/util/vector.h>

/* function declarations (util/list.c): */

PyObject* PyList_FromVector (const Vector *x);

PyObject* PyList_FromMatrix (const Matrix *A);

Vector* PySequence_AsVector (PyObject *pyseq);

Matrix* PySequence_AsMatrix (PyObject *pyseq);

int Vector_Converter (PyObject *obj, void *addr);

int Matrix_Converter (PyObject *obj, void *addr);

#endif /* !__VFL_LIST_H__ */

