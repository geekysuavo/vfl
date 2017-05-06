
/* include the required headers. */
#include <vfl/lang/ast.h>
#include <vfl/vfl.h>

/* eval_int(): evaluate an integer literal node.
 */
static object_t *eval_int (ast_t *node) {
  /* return a new integer object. */
  return (object_t*) int_alloc_with_value(node->n_int.value);
}

/* eval_float(): evaluate a float literal node.
 */
static object_t *eval_float (ast_t *node) {
  /* return a new float object. */
  return (object_t*) float_alloc_with_value(node->n_float.value);
}

/* eval_string(): evaluate a string literal node.
 */
static object_t *eval_string (ast_t *node) {
  /* return a new string object. */
  return (object_t*) string_alloc_with_value(node->n_string.value);
}

/* eval_ident(): evaluate an identifier node.
 */
static object_t *eval_ident (ast_t *node, sym_table_t *tab) {
  /* resolve the specified symbol value. */
  return symbols_get(tab, node->n_string.value);
}

/* eval_ctor(): evaluate a constructor node.
 */
static object_t *eval_ctor (ast_t *node, sym_table_t *tab,
                            const char *name) {
  /* lookup the object type from the name string. */
  const object_type_t *type = vfl_lookup_type(name);
  if (!type)
    return NULL;

  /* allocate an object with the specified type. */
  object_t *obj = obj_alloc(type);
  if (!obj)
    return NULL;

  /* evaluate the passed argument list node. */
  object_t *nodeval = ast_eval(node, tab);
  if (!nodeval) {
    obj_release(obj);
    return NULL;
  }

  /* apply the constructor arguments. */
  map_t *args = (map_t*) nodeval;
  for (size_t i = 0; i < args->len; i++) {
    /* apply the argument as a property. */
    if (!obj_setprop(obj, map_key(args, i), map_val(args, i))) {
      obj_free(obj);
      return NULL;
    }
  }

  /* collect garbage (constructor arguments). */
  obj_collect(args);

  /* return the new object. */
  return obj;
}

/* eval_args(): evaluate an argument list node.
 */
static object_t *eval_args (ast_t *node, sym_table_t *tab) {
  /* allocate a mapping for storing arguments. */
  map_t *map = map_alloc();
  if (!map)
    return NULL;

  /* loop over each argument. */
  for (size_t i = 0; i < node->n_list.len; i++) {
    /* get the current argument node. */
    ast_t *arg = node->n_list.values[i];
    if (!arg)
      continue;

    /* get the argument identifier and expression nodes. */
    ast_t *id = arg->n_binary.left;
    ast_t *ex = arg->n_binary.right;

    /* evaluate the argument expression. */
    object_t *val = ast_eval(ex, tab);
    if (!val)
      goto fail;

    /* get the identifier string. */
    const char *idstr = id->n_string.value;
    if (!map_set(map, idstr, val))
      goto fail;
  }

  /* return the new mapping. */
  return (object_t*) map;

fail:
  /* release the mapping and return null. */
  obj_release((object_t*) map);
  return NULL;
}

/* eval_name(): evaluate a qualified name node.
 */
static object_t *eval_name (ast_t *node, sym_table_t *tab,
                            const int full) {
  /* get the identifier node and its string value. */
  ast_t *id = node->n_binary.left;
  const char *idstr = id->n_string.value;
  if (!id)
    return NULL;

  /* get the qualifier node array. */
  ast_t **quals = node->n_binary.right->n_list.values;
  const size_t n_quals = node->n_binary.right->n_list.len;

  /* check for constructor calls. */
  if (n_quals == 1 && ast_node_type(quals[0]) == AST_NODE_ARGS)
    return eval_ctor(quals[0], tab, idstr);

  /* determine how many qualifiers to resolve. */
  const size_t imax = (full ? n_quals : n_quals - 1);

  /* resolve each qualifier in turn. */
  object_t *val = ast_eval(id, tab);
  for (size_t i = 0; i < imax; i++) {
    /* check if resolution has failed. */
    if (!val)
      return NULL;

    /* get the qualifier node type, and prepare a variable
     * to hold the newly resolved node value.
     */
    const ast_node_type_t qtype = ast_node_type(quals[i]);
    object_t *newval = NULL;

    /* resolve based on the qualifier type. */
    if (qtype == AST_NODE_LIST) {
      /* get the indexed element of the currently resolved object. */
      object_t *idx = ast_eval(quals[i], tab);
      newval = obj_getelem(val, idx);

      /* collect garbage (value and element index). */
      obj_collect(val);
      obj_collect(idx);
    }
    else if (qtype == AST_NODE_IDENT) {
      /* check if the subsequent node (if any) is an argument list. */
      const size_t j = i + 1;
      if (j < n_quals && ast_node_type(quals[j]) == AST_NODE_ARGS) {
        /* call the named method of the currently resolved object. */
        const char *meth = quals[i]->n_string.value;
        object_t *args = ast_eval(quals[j], tab);
        newval = obj_method(val, meth, args);

        /* collect garbage (value and method arguments). */
        obj_collect(val);
        obj_collect(args);

        /* skip the argument list node. */
        i++;
      }
      else {
        /* get the named property of the currently resolved object. */
        const char *prop = quals[i]->n_string.value;
        newval = obj_getprop(val, prop);

        /* collect garbage (value). */
        obj_collect(val);
      }
    }

    /* store the newly resolved value. */
    val = newval;
  }

  /* return the resolved name node object. */
  return val;
}

/* eval_assign(): evaluate an assignment node.
 */
static object_t *eval_assign (ast_t *node, sym_table_t *tab) {
  /* get the two child nodes. */
  ast_t *left = node->n_binary.left;
  ast_t *right = node->n_binary.right;

  /* evaluate the right hand side of the assignment. */
  object_t *val = ast_eval(right, tab);
  if (!val)
    return NULL;

  /* declare a variable to hold the function return value. */
  object_t *result = NULL;

  /* check the type of assignment. */
  if (ast_node_type(left) == AST_NODE_IDENT) {
    /* symbol assignment... set the symbol table entry. */
    const char *var = left->n_string.value;
    if (!symbols_set(tab, var, val))
      return NULL;

    /* return the result. */
    return val;
  }
  else if (ast_node_type(left) == AST_NODE_NAME) {
    /* resolve all but the final qualifier of the name. */
    object_t *obj = eval_name(left, tab, 0);

    /* get the qualifier node array. */
    ast_t **quals = left->n_binary.right->n_list.values;
    const size_t n_quals = left->n_binary.right->n_list.len;

    /* get the type of the final qualifier node. */
    ast_t *qend = quals[n_quals - 1];
    const ast_node_type_t qtype = ast_node_type(qend);

    /* assign based on the node type. */
    if (qtype == AST_NODE_IDENT) {
      /* call the property set method. */
      const char *prop = qend->n_string.value;
      const int ret = obj_setprop(obj, prop, val);

      /* if successful, return a non-null object. */
      if (ret) result = (object_t*) vfl_nil;
    }
    else if (qtype == AST_NODE_LIST) {
      /* call the element set method. */
      object_t *idx = ast_eval(qend, tab);
      const int ret = obj_setelem(obj, idx, val);

      /* collect garbage (element index). */
      obj_collect(idx);

      /* if successful, return a non-null object. */
      if (ret) result = (object_t*) vfl_nil;
    }

    /* collect garbage (target object). */
    obj_collect(obj);
  }

  /* collect garbage (assigned value), and return. */
  obj_collect(val);
  return result;
}

/* eval_arith(): evaluate a binary arithmetic operation.
 */
static object_t *eval_arith (ast_t *node, sym_table_t *tab) {
  /* evaluate the left operand. */
  object_t *left = ast_eval(node->n_binary.left, tab);
  if (!left)
    return NULL;

  /* evaluate the right operand. */
  object_t *right = ast_eval(node->n_binary.right, tab);
  if (!right)
    return NULL;

  /* determine the arithmetic function to call. */
  object_binary_fn fn = NULL;
  switch (ast_node_type(node)) {
    case AST_NODE_ADD: fn = obj_add; break;
    case AST_NODE_SUB: fn = obj_sub; break;
    case AST_NODE_MUL: fn = obj_mul; break;
    case AST_NODE_DIV: fn = obj_div; break;
    case AST_NODE_POW: fn = obj_pow; break;
    default:           fn = NULL;    break;
  }

  /* execute the function. */
  object_t *obj = fn(left, right);

  /* collect garbage (input operands). */
  obj_collect(left);
  obj_collect(right);

  /* return the function result. */
  return obj;
}

/* eval_for(): evaluate a for loop node.
 */
static object_t *eval_for (ast_t *node, sym_table_t *tab) {
  /* get the three child nodes. */
  ast_t *left = node->n_ternary.left;
  ast_t *mid = node->n_ternary.mid;
  ast_t *right = node->n_ternary.right;

  /* evaluate the loop expression. */
  object_t *loopexpr = ast_eval(mid, tab);
  if (!loopexpr)
    return NULL;

  /* declare a variable to hold the function return value. */
  object_t *result = NULL;

  /* check that the loop expression evaluated to a list. */
  if (!OBJECT_IS_LIST(loopexpr))
    goto fail;

  /* loop over the loop expression values. */
  list_t *values = (list_t*) loopexpr;
  const char *var = left->n_string.value;
  for (size_t i = 0; i < values->len; i++) {
    /* set the iteration symbol value. */
    if (!symbols_set(tab, var, list_get(values, i)))
      goto fail;

    /* evaluate the statement block. */
    object_t *obj = ast_eval(right, tab);
    if (!obj)
      goto fail;

    /* collect garbage (statement block value). */
    obj_collect(obj);

    /* unset the iteration symbol value. */
    if (!symbols_set(tab, var, NULL))
      goto fail;
  }

  /* return success. */
  result = (object_t*) vfl_nil;

fail:
  /* collect garbage (loop expression), and return. */
  obj_collect(loopexpr);
  return result;
}

/* eval_list(): evaluate a general list node.
 */
static object_t *eval_list (ast_t *node, sym_table_t *tab) {
  /* allocate a list to store the results. */
  list_t *lst = list_alloc_with_length(node->n_list.len);
  if (!lst)
    return NULL;

  /* loop over the list elements. */
  for (size_t i = 0; i < node->n_list.len; i++) {
    /* evaluate the element. */
    object_t *elem = ast_eval(node->n_list.values[i], tab);
    if (!elem) {
      obj_release((object_t*) lst);
      return NULL;
    }

    /* store the element into the list. */
    list_set(lst, i, elem);
  }

  /* return the new list. */
  return (object_t*) lst;
}

/* eval_block(): evaluate a statement block node.
 */
static object_t *eval_block (ast_t *node, sym_table_t *tab) {
  /* evaluate each statement in the block. */
  for (size_t i = 0; i < node->n_list.len; i++) {
    /* evaluate the current statement. */
    object_t *obj = ast_eval(node->n_list.values[i], tab);
    if (!obj)
      return NULL;

    /* collect garbage (statement value). */
    obj_collect(obj);
  }

  /* return success. */
  VFL_RETURN_NIL;
}

/* --- */

/* ast_eval(): evaluate an abstract syntax tree.
 *
 * arguments:
 *  @node: tree node to evaluate.
 *  @symbols: symbol table of the node.
 *
 * returns:
 *  object holding the final value of the tree, or null
 *  upon evaluation failure.
 */
object_t *ast_eval (ast_t *node, sym_table_t *symbols) {
  /* declare required variables:
   *  @val: current node value object.
   *  @tab: symbol table structure pointer.
   */
  object_t *val = NULL;
  sym_table_t *tab;

  /* check the input pointers. */
  if (!node || !symbols)
    return NULL;

  /* determine how to propagate symbols. */
  switch (ast_node_type(node)) {
    /* new scopes: allocate a new symbol table. */
    case AST_NODE_FOR: tab = symbols_alloc(symbols); break;

    /* other: use the parent symbol table. */
    default: tab = symbols; break;
  }

  /* determine how to traverse from the node. */
  switch (ast_node_type(node)) {
    /* empty nodes. */
    case AST_NODE_EMPTY: break;

    /* literal nodes. */
    case AST_NODE_INT:    val = eval_int(node);    break;
    case AST_NODE_FLOAT:  val = eval_float(node);  break;
    case AST_NODE_STRING: val = eval_string(node); break;

    /* identifier, qualified name, and arguments nodes. */
    case AST_NODE_IDENT: val = eval_ident(node, tab);   break;
    case AST_NODE_NAME:  val = eval_name(node, tab, 1); break;
    case AST_NODE_ARGS:  val = eval_args(node, tab);    break;

    /* assignment nodes. */
    case AST_NODE_ASSIGN: val = eval_assign(node, tab); break;

    /* arithmetic nodes. */
    case AST_NODE_ADD:
    case AST_NODE_SUB:
    case AST_NODE_MUL:
    case AST_NODE_DIV:
    case AST_NODE_POW: val = eval_arith(node, tab); break;

    /* ternary nodes. */
    case AST_NODE_FOR: val = eval_for(node, tab); break;

    /* general list and statement block nodes. */
    case AST_NODE_LIST:  val = eval_list(node, tab);  break;
    case AST_NODE_BLOCK: val = eval_block(node, tab); break;

    /* other. */
    default: break;
  }

  /* if we allocated a new symbol table, free it. */
  if (tab != symbols)
    symbols_free(tab);

  /* return the node value. */
  return val;
}

