#ifndef EXPRESSION_TREE_
#define EXPRESSION_TREE_

#include "../libc/cstring.h"
#include "../libc/errors.h"
#include "../libc/hash_table.h"

typedef struct expression_tree_node {
    String token;
    struct expression_tree_node *left, *right;
} expression_tree_node;

typedef expression_tree_node expression_tree;

err_t expression_tree_init(expression_tree **t);
void expression_tree_free(void *t);

err_t expression_tree_fill_with_data_from_postfix_expression(
    expression_tree **t, const String postfix_exp, hash_table *operators);

void expression_tree_print(expression_tree *t);

#endif  // !EXPRESSION_TREE_
