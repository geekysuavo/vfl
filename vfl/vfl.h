
/* ensure once-only inclusion. */
#ifndef __VFL_VFL_H__
#define __VFL_VFL_H__

/* include the python header. */
#include <Python.h>

/* include vfl inference object headers. */
#include <vfl/datum.h>
#include <vfl/data.h>
#include <vfl/model.h>
#include <vfl/optim.h>
#include <vfl/factor.h>
#include <vfl/search.h>

/* include vfl utility headers. */
#include <vfl/util/list.h>
#include <vfl/util/size_t.h>

/* function declarations: */

int vfl_base_init (PyObject *self, PyObject *args, PyObject *kwargs);

#endif /* !__VFL_VFL_H__ */

