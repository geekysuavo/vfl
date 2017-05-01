
/* ensure once-only inclusion. */
#ifndef __VFL_SYMBOLS_H__
#define __VFL_SYMBOLS_H__

/* include c library headers. */
#include <string.h>

/* include vfl headers. */
#include <vfl/lang/object.h>

/* sym_table_t: defined type for symbol tables. */
typedef struct sym_table sym_table_t;

/* sym_t: structure for holding symbol table entries.
 */
typedef struct {
  /* symbol properties:
   *  @name: symbol string name.
   *  @val: symbol object value.
   */
  char *name;
  object_t *val;
}
sym_t;

/* struct sym_table: structure for holding the symbol table of a
 * current variable scope within an abstract syntax tree.
 */
struct sym_table {
  /* linkages to adjacent symbol tables:
   *  @up: symbol table of the next higher scope.
   */
  sym_table_t *up;

  /* symbol table contents:
   *  @syms: array of symbols.
   *  @n: size of the symbol array.
   */
  sym_t *syms;
  size_t n;
};

/* function declarations (lang/symbols.c): */

sym_table_t *symbols_alloc (sym_table_t *up);

void symbols_free (sym_table_t *tab);

object_t *symbols_get (sym_table_t *tab, const char *name);

int symbols_set (sym_table_t *tab, const char *name, object_t *obj);

#endif /* !__VFL_SYMBOLS_H__ */

