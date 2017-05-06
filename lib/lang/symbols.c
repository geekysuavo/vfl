
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

  /* loop over each symbol in the array. */
  for (size_t i = 0; i < tab->n; i++) {
    /* free the symbol name. */
    free(tab->syms[i].name);

    /* release the symbol value. */
    obj_release(tab->syms[i].val);
  }

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
  if (!tab || !name)
    return 0;

  /* search for the symbol in the table. */
  for (size_t i = 0; i < tab->n; i++) {
    /* on match, set the symbol value. */
    if (strcmp(tab->syms[i].name, name) == 0 &&
        tab->syms[i].val != obj) {
      /* release the current symbol value. */
      obj_release(tab->syms[i].val);

      /* set the new symbol value. */
      tab->syms[i].val = obj;
      obj_retain(obj);
      return 1;
    }
  }

  /* no match. reallocate the symbol array. */
  const size_t n = tab->n;
  const size_t sz = (n + 1) * sizeof(sym_t);
  sym_t *syms = realloc(tab->syms, sz);

  /* allocate the symbol name string. */
  char *dupname = malloc(strlen(name) + 1);

  /* handle allocation failures. */
  if (!syms || !dupname)
    return 0;

  /* store the new symbol information. */
  strcpy(dupname, name);
  syms[n].name = dupname;
  syms[n].val = obj;
  obj_retain(obj);

  /* store the new symbols array. */
  tab->n = n + 1;
  tab->syms = syms;

  /* return success. */
  return 1;
}

