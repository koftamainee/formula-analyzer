#include "expression_tree.h"

#include <stdlib.h>

#include "../libc/logger.h"
#include "../libc/stack.h"
#include "postfix_notation.h"

err_t expression_tree_init(expression_tree **t) {
    if (t == NULL) {
        log_error("passed ptr is NULL");
        return DEREFERENCING_NULL_PTR;
    }

    *t = (expression_tree *)malloc(sizeof(expression_tree));
    if (*t == NULL) {
        log_error("failed to allocatte memory");
        return MEMORY_ALLOCATION_ERROR;
    }
    expression_tree *tree = *t;
    tree->left = NULL;
    tree->right = NULL;
    tree->token = string_init();
    if (tree->token == NULL) {
        log_error("failed to allocatte memory");
        free(tree);
        return MEMORY_ALLOCATION_ERROR;
    }

    return EXIT_SUCCESS;
}

void expression_tree_free(void *tree) {
    if (tree == NULL) {
        return;
    }
    expression_tree_node *t = tree;

    expression_tree_free(t->left);
    expression_tree_free(t->right);
    string_free(t->token);
    free(t);
}

err_t expression_tree_fill_with_data_from_postfix_expression(
    expression_tree **t, const String postfix_exp, hash_table *operators) {
    if (t == NULL || postfix_exp == NULL || operators == NULL) {
        log_error("Passed ptr is NULL");
        return DEREFERENCING_NULL_PTR;
    }

    size_t i = 0;
    err_t err = 0;
    char pe = 0;
    String token = NULL, for_hash_table = NULL;
    stack *st = NULL;
    operator_t *op = NULL;
    int operand_1 = 0, operand_2 = 0, *get_from_hash_table = NULL, result = 0;
    stack_item *p_for_stack = NULL;

    err = stack_init(&st, sizeof(int), free);
    if (err) {
        log_error("error during stack initialization");
        return err;
    }

    token = string_init();
    if (token == NULL) {
        log_error("failed to allocate memory for token");
        stack_free(st);
        return MEMORY_ALLOCATION_ERROR;
    }

    for (i = 0; i < string_len(postfix_exp); ++i) {
        pe = postfix_exp[i];

        // border principle
        if (pe == ' ') {
            if (string_len(token) == 0) {
                continue;
            }
            op = NULL;
            err = hash_table_get(operators, &token, (void **)&op);
            if (err != EXIT_SUCCESS && err != KEY_NOT_FOUND) {
                log_error("Error while getting elem from hash table");
                stack_free(st);
                string_free(token);
                return err;
            }

            if (err == KEY_NOT_FOUND) {  // not an operator, is operand
                log_trace("is operand");
            } else {  // operator
                log_trace("is operator");
            }

            string_free(token);
            token = NULL;
            token = string_init();
            if (token == NULL) {
                log_error("failed to allocate memory for string");
                stack_free(st);
                string_free(token);
                return MEMORY_ALLOCATION_ERROR;
            }
        } else {
            err = string_add(&token, pe);
            if (err) {
                log_error("Error while adding to string");
                stack_free(st);
                string_free(token);
                return err;
            }
        }
    }

    stack_free(st);
    string_free(token);

    return EXIT_SUCCESS;
}

void expression_tree_print(expression_tree *t) {
    if (t == NULL) {
        return;
    }

    expression_tree_print(t->left);
    string_print(t->token);
    printf("\n");
    expression_tree_print(t->right);
}
