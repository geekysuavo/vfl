
/* include the vfl header. */
#include <vfl/vfl.h>

/* include the syntax tree and symbol table headers. */
#include <vfl/lang/ast.h>
#include <vfl/lang/symbols.h>

/* include the dynamic library functions header. */
#include <dlfcn.h>

/* vfl_nil: address of the vfl_nilstruct structure. */
static object_t vfl_nilstruct;
const object_t *vfl_nil = &vfl_nilstruct;

/* nil_type: empty object type structure.
 */
static object_type_t nil_type = {
  "nil", sizeof(object_t),       /* name, size               */
  NULL, NULL, NULL,              /* init, copy, free         */
  NULL, NULL, NULL, NULL, NULL,  /* add, sub, mul, div, pow  */
  NULL, NULL, NULL, NULL         /* get, set, props, methods */
};

/* vfl_object_nil: address of the nil_type structure. */
const object_type_t *vfl_object_nil = &nil_type;

/* --- */

/* modules: global array of dynamically loaded module handles.
 */
static void **modules;
static unsigned int n_modules;

/* object_types: global registry of all recognized object types.
 */
static object_type_t **object_types;
static unsigned int n_object_types;

/* tree, globals: currently stored program and global symbol table.
 */
static sym_table_t *globals;
static ast_t *tree;

/* scanner function declarations. */
int vfl_parse_string (const char *str);
int vfl_parse_file (FILE *fh);

/* vfl_init(): initialize the central registries of object types,
 * and register the core set of models, optimizers, factors,
 * and utility types.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int vfl_init (void) {
  /* declare required variables:
   *  @res: combined result of registering core vfl types.
   */
  int res = 1;

  /* initialize the nil object type. */
  vfl_nilstruct.type = &nil_type;
  vfl_nilstruct.refs = 0;

  /* initialize the module array. */
  modules = NULL;
  n_modules = 0;

  /* initialize the type registry. */
  object_types = NULL;
  n_object_types = 0;

  /* initialize the current program. */
  tree = NULL;

  /* initialize the global symbol table. */
  globals = symbols_alloc(NULL);
  if (!globals)
    return 0;

  /* register the nil object with the table. */
  res &= symbols_set(globals, "nil", (object_t*) vfl_nil);

  /* register true and false with the table. */
  res &= symbols_set(globals, "true", (object_t*) int_alloc_with_value(1));
  res &= symbols_set(globals, "false", (object_t*) int_alloc_with_value(0));

  /* register the math and standard method libraries with the table. */
  res &= symbols_set(globals, "math", math_alloc());
  res &= symbols_set(globals, "std", std_alloc());

  /* register core model types. */
  res &= vfl_register_type((object_type_t*) vfl_model_vfc);
  res &= vfl_register_type((object_type_t*) vfl_model_vfr);
  res &= vfl_register_type((object_type_t*) vfl_model_tauvfr);

  /* register core optimizer types. */
  res &= vfl_register_type((object_type_t*) vfl_optim_fg);
  res &= vfl_register_type((object_type_t*) vfl_optim_mf);

  /* register core factor types. */
  res &= vfl_register_type((object_type_t*) vfl_factor_cosine);
  res &= vfl_register_type((object_type_t*) vfl_factor_decay);
  res &= vfl_register_type((object_type_t*) vfl_factor_fixed_impulse);
  res &= vfl_register_type((object_type_t*) vfl_factor_impulse);
  res &= vfl_register_type((object_type_t*) vfl_factor_polynomial);
  res &= vfl_register_type((object_type_t*) vfl_factor_product);

  /* register utility object types. */
  res &= vfl_register_type(vfl_object_rng);
  res &= vfl_register_type(vfl_object_data);
  res &= vfl_register_type(vfl_object_datum);
  res &= vfl_register_type(vfl_object_list);
  res &= vfl_register_type(vfl_object_map);
  res &= vfl_register_type(vfl_object_int);
  res &= vfl_register_type(vfl_object_float);
  res &= vfl_register_type(vfl_object_string);
  res &= vfl_register_type(vfl_object_search);

  /* return the result. */
  return res;
}

/* vfl_cleanup(): free all memory associated with the central
 * type registry, interpreter, and parser.
 */
void vfl_cleanup (void) {
  /* free the syntax tree. */
  ast_free(tree);
  tree = NULL;

  /* free the global symbol table. */
  symbols_free(globals);
  globals = NULL;

  /* close all dynamically loaded modules. */
  for (unsigned int i = 0; i < n_modules; i++)
    dlclose(modules[i]);

  /* free the modules array. */
  free(modules);
  modules = NULL;
  n_modules = 0;

  /* free the object type array. */
  free(object_types);
  object_types = NULL;
  n_object_types = 0;
}

/* vfl_register_type(): store a new object type in the
 * central type registry.
 *
 * arguments:
 *  @type: object type structure pointer.
 *
 * returns:
 *  integer indicating registration success (1) or failure (0).
 */
int vfl_register_type (const object_type_t *type) {
  /* check the type structure pointer. */
  if (!type)
    return 0;

  /* resize the object type array. */
  n_object_types++;
  const unsigned int bytes = n_object_types * sizeof(object_type_t*);
  object_types = realloc(object_types, bytes);
  if (!object_types)
    return 0;

  /* store the new type and return success. */
  object_types[n_object_types - 1] = (object_type_t*) type;
  return 1;
}

/* vfl_lookup_type(): locate the registered object type structure
 * pointer associated with a given type name, if any.
 *
 * arguments:
 *  @name: object type name string to search for.
 *
 * returns:
 *  object type pointer, or null.
 */
object_type_t *vfl_lookup_type (const char *name) {
  /* check the input string. */
  if (!name)
    return NULL;

  /* loop over the object type registry. */
  for (unsigned int i = 0; i < n_object_types; i++) {
    /* on match, return the type structure pointer. */
    if (strcmp(object_types[i]->name, name) == 0)
      return object_types[i];
  }

  /* no match, return null. */
  return NULL;
}

/* vfl_exec_file(): interpret the contents of a file handle.
 *
 * arguments:
 *  @fh: file handle to interpret.
 *
 * returns:
 *  integer indicating execution success (1) or failure (0).
 */
int vfl_exec_file (FILE *fh) {
  /* check the input argument. */
  if (!fh)
    return 0;

  /* parse the file. */
  if (!vfl_parse_file(fh))
    return 0;

  /* evaluate the resulting syntax tree. */
  if (!ast_eval(tree, globals))
    return 0;

  /* return success. */
  return 1;
}

/* vfl_exec_path(): interpret the contents of a file, specified
 * using a filename.
 *
 * arguments:
 *  @path: path name of the file to interpret.
 *
 * returns:
 *  integer indicating execution success (1) or failure (0).
 */
int vfl_exec_path (const char *fname) {
  /* check the input argument. */
  if (!fname)
    return 0;

  /* open the file. */
  FILE *fh = fopen(fname, "r");
  if (!fh)
    return 0;

  /* parse and close the file. */
  const int ret = vfl_parse_file(fh);
  fclose(fh);

  /* check for parse failures. */
  if (!ret)
    return 0;

  /* evaluate the resulting syntax tree. */
  if (!ast_eval(tree, globals))
    return 0;

  /* return success. */
  return 1;
}

/* vfl_exec_string(): interpret the contents of a string.
 *
 * arguments:
 *  @str: string value to interpret.
 *
 * returns:
 *  integer indicating execution success (1) or failure (0).
 */
int vfl_exec_string (const char *str) {
  /* check the input argument. */
  if (!str)
    return 0;

  /* parse the string. */
  if (!vfl_parse_string(str))
    return 0;

  /* evaluate the resulting syntax tree. */
  if (!ast_eval(tree, globals))
    return 0;

  /* return success. */
  return 1;
}

/* vfl_import(): dynamically import a vfl module at runtime.
 *
 * modules are simply shared libraries with an extra function
 * for registering new types with the central registry. for
 * example, the "foo" module would be a library in the search
 * path with a "int foo_init (void)" function.
 *
 * arguments:
 *  @modname: module name to load.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int vfl_import (const char *modname) {
  /* declare required variables:
   *  @libname: shared library filename string.
   *  @symname: initialization function string.
   *  @res: resulting status code.
   */
  char *libname, *symname;
  int (*initfunc) (void);
  void *lib, *sym;
  int res = 0;

  /* check the input string. */
  if (!modname)
    return 0;

  /* allocate the shared object filename string. */
  libname = malloc(strlen(modname) + 7);
  if (!libname)
    return 0;

  /* allocate the initialization function string. */
  symname = malloc(strlen(modname) + 6);
  if (!symname) {
    free(libname);
    return 0;
  }

  /* build the shared object filename. */
  strcpy(libname, "lib");
  strcat(libname, modname);
  strcat(libname, ".so");

  /* build the initialization function string. */
  strcpy(symname, modname);
  strcat(symname, "_init");

  /* attempt to load the library into memory. */
  lib = dlopen(libname, RTLD_LAZY);
  if (lib) {
    /* reallocate the module array. */
    const size_t szmod = (n_modules + 1) * sizeof(void*);
    void **remod = realloc(modules, szmod);

    /* if reallocation worked, store the new handle. */
    if (remod) {
      n_modules++;
      modules = remod;
      modules[n_modules - 1] = lib;
    }

    /* attempt to resolve the initialization function. */
    sym = dlsym(lib, symname);
    if (sym) {
      /* execute the initialization function. */
      initfunc = sym;
      res = initfunc();
    }
  }

  /* free the allocated strings. */
  free(libname);
  free(symname);

  /* return the result. */
  return res;
}

/* vfl_set_tree(): set the abstract syntax tree to be parsed.
 *
 * arguments:
 *  @node: new abstract syntax tree.
 */
void vfl_set_tree (ast_t *node) {
  /* free the existing syntax tree. */
  ast_free(tree);
  tree = NULL;

  /* set the current syntax tree to the specified node. */
  tree = node;
}

