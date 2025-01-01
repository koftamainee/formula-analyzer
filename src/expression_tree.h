#ifndef EXPRESSION_TREE_
#define EXPRESSION_TREE_

#include "../libc/cstring.h"
#include "../libc/errors.h"

typedef struct expression_tree_node {
    String token;
    struct expression_tree_node *left, *right;
} expression_tree_node;

typedef expression_tree_node expression_tree;

err_t expression_tree_init(expression_tree **t);
void expression_tree_free(expression_tree *t);

err_t expression_tree_fill_with_data_from_postfix_expression(
    expression_tree **t, const String postfix_exp,
    int (*is_op_binary)(const String op));

void expression_tree_print(expression_tree *t);

#endif  // !EXPRESSION_TREE_
