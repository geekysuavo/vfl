
/* ensure once-only inclusion. */
#ifndef __VFL_AST_H__
#define __VFL_AST_H__

/* include vfl headers. */
#include <vfl/lang/object.h>

/* ast_node_type(): macro for accessing the node type of
 * an abstract syntax tree node structure pointer.
 */
#define ast_node_type(T) \
  (T)->base.type

/* ast_t: defined type for abstract syntax trees. */
typedef union ast ast_t;

typedef enum {
  /* literal nodes. */
  AST_NODE_INT,
  AST_NODE_FLOAT,
  AST_NODE_STRING,
  AST_NODE_IDENT,

  /* unary nodes. */
  AST_NODE_MEMBER,
  AST_NODE_METHOD,
  AST_NODE_ELEMENT,

  /* binary nodes. */
  AST_NODE_ASSIGN,
  AST_NODE_ADD,
  AST_NODE_SUB,
  AST_NODE_MUL,
  AST_NODE_DIV,
  AST_NODE_ARG,
  AST_NODE_NAME,

  /* ternary nodes. */
  AST_NODE_FOR,

  /* list nodes. */
  AST_NODE_LIST,
  AST_NODE_ARGS,
  AST_NODE_BLOCK,
  AST_NODE_QUALS
}
ast_node_type_t;

/* ast_node_t: empty nodes.
 */
typedef struct {
  ast_node_type_t type;
}
ast_node_t;

/* ast_node_int_t: integer literal nodes.
 */
typedef struct {
  ast_node_type_t type;
  long value;
}
ast_node_int_t;

/* ast_node_float_t: float literal nodes.
 */
typedef struct {
  ast_node_type_t type;
  double value;
}
ast_node_float_t;

/* ast_node_string_t: string nodes, both literal and identifier.
 */
typedef struct {
  ast_node_type_t type;
  char *value;
}
ast_node_string_t;

/* ast_node_unary_t: nodes with exactly one child.
 */
typedef struct {
  ast_node_type_t type;
  ast_t *sub;
}
ast_node_unary_t;

/* ast_node_binary_t: nodes with exactly two children.
 */
typedef struct {
  ast_node_type_t type;
  ast_t *left, *right;
}
ast_node_binary_t;

/* ast_node_ternary_t: nodes with exactly three children.
 */
typedef struct {
  ast_node_type_t type;
  ast_t *left, *mid, *right;
}
ast_node_ternary_t;

/* ast_node_list_t: nodes with one or more children.
 */
typedef struct {
  ast_node_type_t type;
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

/* function declarations (lang/ast.c): */

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

#endif /* !__VFL_AST_H__ */

