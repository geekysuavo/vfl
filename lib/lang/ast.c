
/* include the abstract syntax tree header. */
#include <vfl/lang/ast.h>

/* ast_alloc(): allocate a blank abstract syntax tree node.
 *
 * returns:
 *  newly allocated and initialized (at the base level)
 *  abstract syntax tree node, or null on failure.
 */
static ast_t *ast_alloc (void) {
  /* allocate a new node. */
  ast_t *node = malloc(sizeof(ast_t));
  if (!node)
    return NULL;

  /* initialize the node type, table, value and scope. */
  ast_node_type(node) = AST_NODE_EMPTY;
  ast_node_table(node) = NULL;
  ast_node_value(node) = NULL;
  ast_node_scope(node) = 0;

  /* return the new node. */
  return node;
}

/* ast_int(): allocate a new integer literal node.
 *
 * arguments:
 *  @value: value of the node.
 *
 * returns:
 *  newly allocated tree node, or null on failure.
 */
ast_t *ast_int (const long value) {
  /* allocate a new node. */
  ast_t *node = ast_alloc();
  if (!node)
    return NULL;

  /* set the node type and value. */
  ast_node_type(node) = AST_NODE_INT;
  node->n_int.value = value;

  /* return the new node. */
  return node;
}

/* ast_float(): allocate a new float literal node.
 *
 * arguments:
 *  @value: value of the node.
 *
 * returns:
 *  newly allocated tree node, or null on failure.
 */
ast_t *ast_float (const double value) {
  /* allocate a new node. */
  ast_t *node = ast_alloc();
  if (!node)
    return NULL;

  /* set the node type and value. */
  ast_node_type(node) = AST_NODE_FLOAT;
  node->n_float.value = value;

  /* return the new node. */
  return node;
}

/* ast_string(): allocate a new string, identifier, or name node.
 *
 * arguments:
 *  @type: type of the node.
 *  @value: value of the node.
 *
 * returns:
 *  newly allocated tree node, or null on failure.
 */
ast_t *ast_string (const ast_node_type_t type, const char *value) {
  /* allocate a new node. */
  ast_t *node = ast_alloc();
  if (!node)
    return NULL;

  /* set the node type and value. */
  ast_node_type(node) = type;
  node->n_string.value = (char*) value;

  /* return the new node. */
  return node;
}

/* ast_binary(): allocate a new binary node.
 *
 * arguments:
 *  @type: type of the node.
 *  @left: first child for the node.
 *  @right: second child for the node.
 *
 * returns:
 *  newly allocated tree node, or null on failure.
 */
ast_t *ast_binary (const ast_node_type_t type,
                   ast_t *left, ast_t *right) {
  /* allocate a new node. */
  ast_t *node = ast_alloc();
  if (!node)
    return NULL;

  /* set the node type and children. */
  ast_node_type(node) = type;
  node->n_binary.left = left;
  node->n_binary.right = right;

  /* return the new node. */
  return node;
}

/* ast_ternary(): allocate a new ternary node.
 *
 * arguments:
 *  @type: type of the node.
 *  @left: first child for the node.
 *  @mid: second child for the node.
 *  @right: third child for the node.
 *
 * returns:
 *  newly allocated tree node, or null on failure.
 */
ast_t *ast_ternary (const ast_node_type_t type,
                    ast_t *left, ast_t *mid, ast_t *right) {
  /* allocate a new node. */
  ast_t *node = ast_alloc();
  if (!node)
    return NULL;

  /* set the node type and children. */
  ast_node_type(node) = AST_NODE_FOR;
  node->n_ternary.left = left;
  node->n_ternary.mid = mid;
  node->n_ternary.right = right;

  /* return the new node. */
  return node;
}

/* ast_list(): allocate a new list (n-ary) node.
 *
 * arguments:
 *  @type: type of the node.
 *  @first: first child of the node.
 *
 * returns:
 *  newly allocated tree node, or null on failure.
 */
ast_t *ast_list (const ast_node_type_t type, ast_t *first) {
  /* allocate a new node. */
  ast_t *node = ast_alloc();
  if (!node)
    return NULL;

  /* set the node type. */
  ast_node_type(node) = type;

  /* allocate the child node array. */
  ast_t **values = malloc(sizeof(ast_t*));

  /* handle allocation failure. */
  if (!values) {
    free(node);
    return NULL;
  }

  /* store the new child node. */
  node->n_list.len = 1;
  node->n_list.values = values;
  node->n_list.values[0] = first;

  /* return the new node. */
  return node;
}

/* ast_list_append(): append a child node to a list node.
 *
 * arguments:
 *  @node: parent list node to append to.
 *  @next: next child of the node.
 *
 * returns:
 *  modified tree node, or null on failure.
 */
ast_t *ast_list_append (ast_t *node, ast_t *next) {
  /* check the input node. */
  if (!node)
    return NULL;

  /* reallocate the child node array. */
  const size_t n = node->n_list.len;
  const size_t sz = (n + 1) * sizeof(ast_t*);
  ast_t **values = realloc(node->n_list.values, sz);

  /* handle reallocation failure. */
  if (!values) {
    ast_free(node);
    return NULL;
  }

  /* store the new child node. */
  node->n_list.len = n + 1;
  node->n_list.values = values;
  node->n_list.values[n] = next;

  /* return the modified node. */
  return node;
}

/* ast_free(): free an abstract syntax tree.
 *
 * arguments:
 *  @node: tree node to free, along with all children.
 */
void ast_free (ast_t *node) {
  /* return if the node pointer is null. */
  if (!node)
    return;

  /* act based on node type. */
  switch (ast_node_type(node)) {
    /* string nodes. */
    case AST_NODE_STRING:
    case AST_NODE_IDENT:
      /* free the string value. */
      free(node->n_string.value);
      break;

    /* binary nodes. */
    case AST_NODE_ASSIGN:
    case AST_NODE_ADD:
    case AST_NODE_SUB:
    case AST_NODE_MUL:
    case AST_NODE_DIV:
    case AST_NODE_ARG:
    case AST_NODE_NAME:
      /* free the child nodes. */
      ast_free(node->n_binary.left);
      ast_free(node->n_binary.right);
      break;

    /* ternary nodes. */
    case AST_NODE_FOR:
      /* free the child nodes. */
      ast_free(node->n_ternary.left);
      ast_free(node->n_ternary.mid);
      ast_free(node->n_ternary.right);
      break;

    /* list nodes. */
    case AST_NODE_LIST:
    case AST_NODE_ARGS:
    case AST_NODE_BLOCK:
    case AST_NODE_QUALS:
      /* free all child nodes. */
      for (size_t i = 0; i < node->n_list.len; i++)
        ast_free(node->n_list.values[i]);

      /* free the child node array. */
      free(node->n_list.values);
      break;

    /* other. */
    default:
      break;
  }

  /* if the node owns its symbol table, free it. */
  if (ast_node_scope(node))
    symbols_free(ast_node_table(node));

  /* free the node structure. */
  free(node);
}

