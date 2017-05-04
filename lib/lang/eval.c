
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

  /* get the right hand side value. */
  object_t *val = ast_node_value(right);

  /* check the type of assignment. */
  if (ast_node_type(left) == AST_NODE_IDENT) {
    /* symbol assignment... set the symbol table entry. */
    if (!symbols_set(tab, left->n_string.value, val))
      return 0;
  }
  else if (ast_node_type(left) == AST_NODE_NAME) {
    /* qualified assignment... get the qualifier node array. */
    ast_t **quals = left->n_binary.right->n_list.values;
    const size_t n_quals = left->n_binary.right->n_list.len;

    /* get the object used for calling setters. */
    object_t *obj = NULL;
    if (n_quals == 1)
      obj = ast_node_value(left->n_binary.left);
    else
      obj = ast_node_value(quals[n_quals - 2]);

    /* determine the type of qualified assignment. */
    ast_t *qend = quals[n_quals - 1];
    if (ast_node_type(qend) == AST_NODE_IDENT) {
      /* call the property set method. */
      const char *propname = qend->n_string.value;
      if (!obj_setprop(obj, propname, val))
        return 0;
    }
    else if (ast_node_type(qend) == AST_NODE_LIST) {
      /* re-evaluate the index node. */
      if (!ast_eval(qend, tab))
        return 0;

      /* call the element set method. */
      object_t *idx = ast_node_value(qend);
      if (!obj_setelem(obj, idx, val))
        return 0;
    }
    else
      return 0;
  }
  else
    return 0;

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

/* ast_eval_ctor(): evaluate a constructor node.
 */
static int ast_eval_ctor (ast_t *node, sym_table_t *tab,
                          const char *name, map_t *args) {
  /* lookup the object type from the name string. */
  const object_type_t *type = vfl_lookup_type(name);
  if (!type)
    return 0;

  /* allocate an object with the specified type. */
  object_t *obj = obj_alloc(type);
  if (!obj)
    return 0;

  /* apply the constructor arguments. */
  for (size_t i = 0; i < args->len; i++) {
    /* apply the argument as a property. */
    if (!obj_setprop(obj, map_key(args, i), map_val(args, i)))
      return 0;
  }

  /* set the node value and return success. */
  ast_node_value(node) = obj;
  return 1;
}

/* ast_eval_name(): evaluate a qualified name node.
 */
static int ast_eval_name (ast_t *node, sym_table_t *tab) {
  /* get the identifier node. */
  ast_t *id = node->n_binary.left;
  const char *idstr = id->n_string.value;

  /* get the qualifier array and count. */
  ast_t **quals = node->n_binary.right->n_list.values;
  const size_t n_quals = node->n_binary.right->n_list.len;

  /* check for constructor calls. */
  if (n_quals == 1 && ast_node_type(quals[0]) == AST_NODE_ARGS) {
    /* evaluate the argument list. */
    if (!ast_eval(quals[0], tab))
      return 0;

    /* evaluate the constructor. */
    map_t *map = (map_t*) ast_node_value(quals[0]);
    return ast_eval_ctor(node, tab, idstr, map);
  }

  /* evaluate the identifier node. */
  if (!ast_eval(id, tab))
    return 0;

  /* initialize the resolved node value. */
  object_t *obj = ast_node_value(id);

  /* loop over the qualifiers. */
  for (size_t i = 0; i < n_quals; i++) {
    /* get the qualifier node type. */
    const ast_node_type_t qtype = ast_node_type(quals[i]);
    if (qtype == AST_NODE_LIST) {
      /* evaluate the element index. */
      if (!ast_eval(quals[i], tab))
        return 0;

      /* get the element from the currently resolved object. */
      object_t *idx = ast_node_value(quals[i]);
      obj = obj_getelem(obj, idx);
      if (!obj)
        return 0;
    }
    else if (qtype == AST_NODE_IDENT) {
      /* check if the next node is an argument list. */
      const size_t j = i + 1;
      if (j < n_quals && ast_node_type(quals[j]) == AST_NODE_ARGS) {
        /* evaluate the argument list. */
        if (!ast_eval(quals[j], tab))
          return 0;

        /* call the method of the currently resolved object. */
        const char *methname = quals[i]->n_string.value;
        obj = obj_method(obj, methname, ast_node_value(quals[j]));
        if (!obj)
          return 0;

        /* skip the argument list. */
        i++;
      }
      else {
        /* get the property from the currently resolved object. */
        const char *propname = quals[i]->n_string.value;
        obj = obj_getprop(obj, propname);
        if (!obj)
          return 0;
      }
    }

    /* store the qualifier node value. */
    ast_node_value(quals[i]) = obj;
  }

  /* return success. */
  ast_node_value(node) = obj;
  return 1;
}

/* ast_eval_for(): evaluate a for loop node.
 */
static int ast_eval_for (ast_t *node, sym_table_t *tab) {
  /* get the three child nodes. */
  ast_t *left = node->n_ternary.left;
  ast_t *mid = node->n_ternary.mid;
  ast_t *right = node->n_ternary.right;

  /* evaluate the middle operand (loop values). */
  if (!ast_eval(mid, tab))
    return 0;

  /* check that the loop values are now available as a list. */
  object_t *obj = ast_node_value(mid);
  if (!obj || !OBJECT_IS_LIST(obj))
    return 0;

  /* loop over the values of the iteration variable. */
  list_t *values = (list_t*) obj;
  const char *var = left->n_string.value;
  for (size_t i = 0; i < values->len; i++) {
    /* set the iteration symbol value. */
    if (!symbols_set(tab, var, list_get(values, i)))
      return 0;

    /* evaluate the statement block. */
    if (!ast_eval(right, tab))
      return 0;
  }

  /* return success. */
  return 1;
}

/* ast_eval_list(): evaluate a list node.
 */
static int ast_eval_list (ast_t *node, sym_table_t *tab) {
  /* loop over each list element. */
  for (size_t i = 0; i < node->n_list.len; i++) {
    /* evaluate the element. */
    if (!ast_eval(node->n_list.values[i], tab))
      return 0;
  }

  /* allocate a list object. */
  list_t *lst = list_alloc_with_length(node->n_list.len);
  if (!lst)
    return 0;

  /* store the list elements. */
  for (size_t i = 0; i < lst->len; i++)
    list_set(lst, i, ast_node_value(node->n_list.values[i]));

  /* store the node value and return success. */
  ast_node_value(node) = (object_t*) lst;
  return 1;
}

/* ast_eval_args(): evaluate an argument list block.
 */
static int ast_eval_args (ast_t *node, sym_table_t *tab) {
  /* allocate a mapping for storing arguments. */
  map_t *map = map_alloc();
  if (!map)
    return 0;

  /* store the map as the node value. */
  ast_node_value(node) = (object_t*) map;

  /* loop over each argument. */
  for (size_t i = 0; i < node->n_list.len; i++) {
    /* get the argument node. */
    ast_t *arg = node->n_list.values[i];
    if (!arg)
      continue;

    /* get the argument identifier and expression nodes. */
    ast_t *id = arg->n_binary.left;
    ast_t *ex = arg->n_binary.right;

    /* evaluate the expression. */
    if (!ast_eval(ex, tab))
      return 0;

    /* get the key string and value object. */
    const char *key = id->n_string.value;
    object_t *val = ast_node_value(ex);

    /* store the key-value pair in the map. */
    if (!map_set(map, key, val))
      return 0;
  }

  /* return success. */
  return 1;
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

    /* qualified name nodes. */
    case AST_NODE_NAME:
      if (!ast_eval_name(node, tab)) return 0;
      break;

    /* for loop nodes. */
    case AST_NODE_FOR:
      if (!ast_eval_for(node, tab)) return 0;
      break;

    /* list nodes. */
    case AST_NODE_LIST:
      if (!ast_eval_list(node, tab)) return 0;
      break;

    /* arguments nodes. */
    case AST_NODE_ARGS:
      if (!ast_eval_args(node, tab)) return 0;
      break;

    /* block nodes. */
    case AST_NODE_BLOCK:
      if (!ast_eval_block(node, tab)) return 0;
      break;

    /* other. */
    default:
      break;
  }

  /* return success on leaf nodes. */
  return 1;
}

