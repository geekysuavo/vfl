
/* ensure once-only inclusion. */
#ifndef __VFL_AST_H__
#define __VFL_AST_H__

/* include vfl headers. */
#include <vfl/lang/symbols.h>

/* ast_node_*(): macro for accessing the base members of
 * any abstract syntax tree node structure pointer.
 */
#define ast_node_type(T)   (T)->base.type
#define ast_node_table(T)  (T)->base.tab
#define ast_node_value(T)  (T)->base.obj
#define ast_node_scope(T)  (T)->base.scope

/* AST_BASE: macro defining the base structure members of all
 * abstract syntax tree nodes.
 */
#define AST_BASE \
  ast_node_type_t type;  /* @type: syntax tree node type.       */  \
  sym_table_t *tab;      /* @tab: current scope symbol table.   */  \
  object_t *obj;         /* @obj: object value of the node.     */  \
  int scope;             /* @scope: if the node owns its table. */

/* ast_t: defined type for abstract syntax trees. */
typedef union ast ast_t;

/* ast_node_type_t: enumeration of all accepted syntax tree node types.
 */
typedef enum {
  /* empty nodes. */
  AST_NODE_EMPTY = 0,

  /* literal nodes. */
  AST_NODE_INT = 100,
  AST_NODE_FLOAT,
  AST_NODE_STRING,
  AST_NODE_IDENT,

  /* unary nodes. */
  AST_NODE_MEMBER = 200,
  AST_NODE_METHOD,
  AST_NODE_ELEMENT,

  /* binary nodes. */
  AST_NODE_ASSIGN = 300,
  AST_NODE_ADD,
  AST_NODE_SUB,
  AST_NODE_MUL,
  AST_NODE_DIV,
  AST_NODE_ARG,
  AST_NODE_NAME,

  /* ternary nodes. */
  AST_NODE_FOR = 400,

  /* list nodes. */
  AST_NODE_LIST = 500,
  AST_NODE_ARGS,
  AST_NODE_BLOCK,
  AST_NODE_QUALS
}
ast_node_type_t;

/* ast_node_t: empty nodes.
 */
typedef struct {
  AST_BASE;
}
ast_node_t;

/* ast_node_int_t: integer literal nodes.
 */
typedef struct {
  AST_BASE;

  /* @value: int value of the node. */
  long value;
}
ast_node_int_t;

/* ast_node_float_t: float literal nodes.
 */
typedef struct {
  AST_BASE;

  /* @value: float value of the node. */
  double value;
}
ast_node_float_t;

/* ast_node_string_t: string nodes, both literal and identifier.
 */
typedef struct {
  AST_BASE;

  /* @value: string value of the node. */
  char *value;
}
ast_node_string_t;

/* ast_node_unary_t: nodes with exactly one child.
 */
typedef struct {
  AST_BASE;

  /* @sub: child node. */
  ast_t *sub;
}
ast_node_unary_t;

/* ast_node_binary_t: nodes with exactly two children.
 */
typedef struct {
  AST_BASE;

  /* @left: first child node.
   * @right: second child node.
   */
  ast_t *left, *right;
}
ast_node_binary_t;

/* ast_node_ternary_t: nodes with exactly three children.
 */
typedef struct {
  AST_BASE;

  /* @left: first child node.
   * @mid: second child node.
   * @right: third child node.
   */
  ast_t *left, *mid, *right;
}
ast_node_ternary_t;

/* ast_node_list_t: nodes with one or more children.
 */
typedef struct {
  AST_BASE;

  /* @values: array of child nodes.
   * @len: size of child node array.
   */
  ast_t **values;
  size_t len;
}
ast_node_list_t;

/* union ast: union of all abstract syntax tree node structures.
 */
union ast {
  /* empty nodes, for accessing node type. */
  ast_node_t base;

  /* literal nodes. */
  ast_node_int_t n_int;
  ast_node_float_t n_float;
  ast_node_string_t n_string;

  /* unary, binary and ternary nodes. */
  ast_node_unary_t n_unary;
  ast_node_binary_t n_binary;
  ast_node_ternary_t n_ternary;

  /* list nodes. */
  ast_node_list_t n_list;
};

/* function declarations, allocation (lang/ast.c): */

ast_t *ast_int (const long value);

ast_t *ast_float (const double value);

ast_t *ast_string (const ast_node_type_t type, const char *value);

ast_t *ast_unary (const ast_node_type_t type, ast_t *sub);

ast_t *ast_binary (const ast_node_type_t type,
                   ast_t *left, ast_t *right);

ast_t *ast_ternary (const ast_node_type_t type,
                    ast_t *left, ast_t *mid, ast_t *right);

ast_t *ast_list (const ast_node_type_t type, ast_t *first);

ast_t *ast_list_append (ast_t *node, ast_t *next);

void ast_free (ast_t *node);

/* function declarations, evaluation (lang/eval.c): */

int ast_eval (ast_t *node, sym_table_t *symbols);

#endif /* !__VFL_AST_H__ */

