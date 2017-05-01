
/* include the required headers. */
#include <vfl/lang/ast.h>
#include <vfl/vfl.h>

/* ast_eval_int(): evaluate an integer literal node.
 */
static int ast_eval_int (ast_t *node) {
  /* set the node value to a new integer object. */
  int_t *iobj = int_alloc_with_value(node->n_int.value);
  ast_node_value(node) = (object_t*) iobj;
  if (!iobj)
    return 0;

  /* return success. */
  return 1;
}

/* ast_eval_float(): evaluate a float literal node.
 */
static int ast_eval_float (ast_t *node) {
  /* set the node value to a new float object. */
  flt_t *fobj = float_alloc_with_value(node->n_float.value);
  ast_node_value(node) = (object_t*) fobj;
  if (!fobj)
    return 0;

  /* return success. */
  return 1;
}

/* ast_eval_string(): evaluate a string literal node.
 */
static int ast_eval_string (ast_t *node) {
  /* set the node value to a new string object. */
  string_t *sobj = string_alloc_with_value(node->n_string.value);
  ast_node_value(node) = (object_t*) sobj;
  if (!sobj)
    return 0;

  /* return success. */
  return 1;
}

/* ast_eval_ident(): evaluate an identifier node.
 */
static int ast_eval_ident (ast_t *node, sym_table_t *tab) {
  /* if possible, retrieve the current symbol value. */
  ast_node_value(node) = symbols_get(tab, node->n_string.value);

  /* return success. */
  return 1;
}

/* ast_eval_assign(): evaluate an assignment node.
 */
static int ast_eval_assign (ast_t *node, sym_table_t *tab) {
  /* get the two child nodes. */
  ast_t *left = node->n_binary.left;
  ast_t *right = node->n_binary.right;

  /* evaluate the left hand side of the assignment. */
  if (!ast_eval(left, tab))
    return 0;

  /* evaluate the right hand side of the assignment. */
  if (!ast_eval(right, tab))
    return 0;

  /* check the type of assignment. */
  if (ast_node_type(left) == AST_NODE_IDENT) {
    /* variable assignment... free the existing node value. */
    obj_free(ast_node_value(left));

    /* set the symbol table entry. */
    if (!symbols_set(tab, left->n_string.value, ast_node_value(right)))
      return 0;
  }
  else {
    /* FIXME: implement qualified assignments. */
  }

  /* store the node value and return success. */
  ast_node_value(node) = ast_node_value(right);
  return 1;
}

/* ast_eval_arith(): evaluate a binary arithmetic operation.
 */
static int ast_eval_arith (ast_t *node, sym_table_t *tab) {
  /* declare required variables:
   *  @fn: binary arithmetic function pointer.
   *  @obj: resulting object value.
   */
  object_binary_fn fn = NULL;
  object_t *obj = NULL;

  /* evaluate the left operand. */
  if (!ast_eval(node->n_binary.left, tab))
    return 0;

  /* evaluate the right operand. */
  if (!ast_eval(node->n_binary.right, tab))
    return 0;

  /* determine the arithmetic function to call. */
  switch (ast_node_type(node)) {
    case AST_NODE_ADD: fn = obj_add; break;
    case AST_NODE_SUB: fn = obj_sub; break;
    case AST_NODE_MUL: fn = obj_mul; break;
    case AST_NODE_DIV: fn = obj_div; break;
    default: fn = NULL; break;
  }

  /* execute the function. */
  obj = fn(ast_node_value(node->n_binary.left),
           ast_node_value(node->n_binary.right));

  /* store the node value and return. */
  ast_node_value(node) = obj;
  return (obj ? 1 : 0);
}

/* ast_eval_block(): evaluate a statement block.
 */
static int ast_eval_block (ast_t *node, sym_table_t *tab) {
  /* loop over each statement within the block. */
  for (size_t i = 0; i < node->n_list.len; i++) {
    /* evaluate the statement. */
    ast_t *sub = node->n_list.values[i];
    if (!ast_eval(sub, tab))
      return 0;

    /* set the block value to that of the (last) statement. */
    ast_node_value(node) = ast_node_value(sub);
  }

  /* return success. */
  return 1;
}

/* --- */

/* ast_eval(): evaluate an abstract syntax tree.
 *
 * arguments:
 *  @node: tree node to evaluate.
 *  @symbols: symbol table of the node.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int ast_eval (ast_t *node, sym_table_t *symbols) {
  /* declare required variables:
   *  @tab: symbol table structure pointer.
   */
  sym_table_t *tab;

  /* check the input pointers. */
  if (!node || !symbols)
    return 0;

  /* determine how to propagate symbols. */
  switch (ast_node_type(node)) {
    /* new scopes. */
    case AST_NODE_FOR:
      /* allocate a new symbol table. */
      tab = symbols_alloc(symbols);
      break;

    /* other. */
    default:
      /* use the parent symbol table. */
      tab = symbols;
      break;
  }

  /* set the symbol table of the current node. */
  ast_node_scope(node) = (tab == symbols ? 0 : 1);
  ast_node_table(node) = tab;

  /* determine how to traverse from the node. */
  switch (ast_node_type(node)) {
    /* empty nodes. */
    case AST_NODE_EMPTY:
      return 0;

    /* integer nodes. */
    case AST_NODE_INT:
      if (!ast_eval_int(node)) return 0;
      break;

    /* float nodes. */
    case AST_NODE_FLOAT:
      if (!ast_eval_float(node)) return 0;
      break;

    /* string nodes. */
    case AST_NODE_STRING:
      if (!ast_eval_string(node)) return 0;
      break;

    /* identifier nodes. */
    case AST_NODE_IDENT:
      if (!ast_eval_ident(node, tab)) return 0;
      break;

    /* assignment nodes. */
    case AST_NODE_ASSIGN:
      if (!ast_eval_assign(node, tab)) return 0;
      break;

    /* arithmetic nodes. */
    case AST_NODE_ADD:
    case AST_NODE_SUB:
    case AST_NODE_MUL:
    case AST_NODE_DIV:
      if (!ast_eval_arith(node, tab)) return 0;
      break;

    /* block nodes (lists). */
    case AST_NODE_BLOCK:
      if (!ast_eval_block(node, tab)) return 0;
      break;

    /* other. */
    default:
/*FIXME*/
fprintf(stderr,"node = 0x%lx:\n", (size_t) node);
fprintf(stderr,"  type  = %u\n", ast_node_type(node));
fprintf(stderr,"  value = 0x%lx\n", (size_t) ast_node_value(node));
fprintf(stderr,"  table = 0x%lx\n", (size_t) ast_node_table(node));
fprintf(stderr,"  scope = %u\n", ast_node_scope(node));
fflush(stderr);
/*FIXME*/
      break;
  }
/*FIXME*/
object_t *obj=ast_node_value(node);
if(obj){
if(OBJECT_IS_INT(obj))
  printf("result[int] = %ld\n",int_get((int_t*)obj));
else if(OBJECT_IS_FLOAT(obj))
  printf("result[float] = %lf\n",float_get((flt_t*)obj));
else if(OBJECT_IS_STRING(obj))
  printf("result[string] = '%s'\n",string_get((string_t*)obj));
}
/*FIXME*/

  /* return success on leaf nodes. */
  return 1;
}

