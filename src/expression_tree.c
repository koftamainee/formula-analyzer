#include "expression_tree.h"

#include <stdlib.h>

#include "../libc/logger.h"
#include "../libc/stack.h"

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

void expression_tree_free(expression_tree *t) {
    if (t == NULL) {
        return;
    }
    expression_tree_free(t->left);
    expression_tree_free(t->right);
    string_free(t->token);
    free(t);
}

err_t expression_tree_fill_with_data_from_postfix_expression(
    expression_tree **t, const String postfix_exp,
    int (*is_op_binary)(const String op)) {
    if (t == NULL || postfix_exp == NULL || is_op_binary == NULL) {
        log_error("Passed ptr is NULL");
        return DEREFERENCING_NULL_PTR;
    }

    size_t i = 0;
    char pe = 0;
    String token = NULL;
    err_t err = 0;
    stack *operands = NULL;
    stack_item *p_for_stack = NULL;
    expression_tree_node *node = NULL;

    token = string_init();
    if (token == NULL) {
        log_error("failed to allocate memory");
        return MEMORY_ALLOCATION_ERROR;
    }

    // err = stack_init(&operands, sizeof(expression_tree_node *),
    //                  expression_tree_string_free);
    // if (err) {
    //     log_error("failed to init stack");
    //     string_free(token);
    //     return err;
    // }

    for (i = 0; i < string_len(postfix_exp); ++i) {
        pe = postfix_exp[i];
        if (pe == ' ') {  // end of lexem

            switch (is_op_binary(token)) {
                case 1:  // operator is binary
                    break;
                case 0:  // operator is unary
                    break;
                case -1:  // is not operator, is operand
                    node = (expression_tree_node *)malloc(
                        sizeof(expression_tree_node));
                    if (node == NULL) {
                        log_error("failed to allocate memory");
                        stack_free(operands);
                        string_free(token);
                        return MEMORY_ALLOCATION_ERROR;
                    }
                    node->token = token;
                    token = NULL;
                    node->right = NULL;
                    node->left = NULL;
                    err = stack_push(operands, node);
                    if (err) {
                        log_error("failed push to stack");
                        stack_free(operands);
                        string_free(token);
                        return err;
                    }
                    break;
            }

            token = NULL;
            node = NULL;
            token = string_init();
            if (token == NULL) {
                log_error("failed to allocate memory");
                stack_free(operands);
                return MEMORY_ALLOCATION_ERROR;
            }

        } else {
            err = string_add(&token, pe);
            if (err) {
                log_error("failed to add to string");
                stack_free(operands);
                string_free(token);
                return err;
            }
        }
    }

    err = stack_top(operands, &p_for_stack);
    if (err != EXIT_SUCCESS && err != STACK_IS_EMPTY) {
        stack_free(operands);
        log_error("failed to top stack");
        return err;
    }
    if (err == STACK_IS_EMPTY) {
        stack_free(operands);
        log_error("Invalid expression, stack is empty");
        return INVALID_OPERATIONS;
    }
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
