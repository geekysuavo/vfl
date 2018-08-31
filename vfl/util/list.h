
/* ensure once-only inclusion. */
#ifndef __VFL_LIST_H__
#define __VFL_LIST_H__

/* include vfl headers. */
#include <vfl/util/vector.h>

/* function declarations (util/list.c): */

PyObject* PyList_FromVector (const Vector *x);

Vector* PySequence_AsVector (PyObject *pyseq);

#endif /* !__VFL_LIST_H__ */

