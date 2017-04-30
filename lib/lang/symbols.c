
/* include the symbol table header. */
#include <vfl/lang/symbols.h>

/* symbols_alloc(): allocate an empty symbol table.
 *
 * arguments:
 *  @up: symbol table of the higher scope, or null.
 *
 * returns:
 *  newly allocated and initialized symbol table.
 */
sym_table_t *symbols_alloc (sym_table_t *up) {
  /* allocate a new symbol table. */
  sym_table_t *tab = malloc(sizeof(sym_table_t));
  if (!tab)
    return NULL;

  /* store the upper table pointer. */
  tab->up = up;

  /* initialize the symbol array. */
  tab->syms = NULL;
  tab->n = 0;

  /* return the new symbol table. */
  return tab;
}

/* symbols_free(): free a symbol table.
 *
 * arguments:
 *  @tab: symbol table structure pointer to free.
 */
void symbols_free (sym_table_t *tab) {
  /* return if the structure pointer is null. */
  if (!tab)
    return;

  /* free the symbol array and the structure. */
  free(tab->syms);
  free(tab);
}

/* symbols_get(): get the object value of a symbol from a symbol
 * table.
 *
 * arguments:
 *  @tab: symbol table to search within.
 *  @name: symbol name to search for.
 *
 * returns:
 *  object value if the symbol was found in the table at or
 *  above the current scope, or null if not.
 */
object_t *symbols_get (sym_table_t *tab, const char *name) {
  /* check the input pointers. */
  if (!tab || !name)
    return NULL;

  /* search for symbols. */
  sym_table_t *scope = tab;
  while (scope) {
    /* search for symbols in the current scope. */
    for (size_t i = 0; i < scope->n; i++) {
      /* on match, return the symbol value. */
      if (strcmp(scope->syms[i].name, name) == 0)
        return scope->syms[i].val;
    }

    /* no match; move to the next higher scope. */
    scope = scope->up;
  }

  /* no match at any scope; return null. */
  return NULL;
}

/* symbols_set(): set the object value of a symbol in a symbol table.
 *
 * arguments:
 *  @tab: symbol table to modify.
 *  @name: symbol name of the object.
 *  @obj: symbol value to store/overwrite.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int symbols_set (sym_table_t *tab, const char *name, object_t *obj) {
  /* check the input pointers. */
  if (!tab || !name || !obj)
    return 0;

  /* FIXME: implement symbols_set() */

  /* return success. */
  return 1;
}

/* symbols_delete(): remove a symbol from a symbol table.
 *
 * arguments:
 *  @tab: symbol table to modify.
 *  @name: symbol name to remove.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int symbols_delete (sym_table_t *tab, const char *name) {
  /* check the input pointers. */
  if (!tab || !name)
    return 0;

  /* FIXME: implement symbols_delete() */

  /* return success. */
  return 1;
}

