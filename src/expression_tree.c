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

void expression_tree_free_single_node(void *tree) {
    if (tree == NULL) {
        return;
    }

    expression_tree_node *t = tree;

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
    String token = NULL;
    stack *st = NULL;
    operator_t *op = NULL;
    expression_tree_node *operand_1 = NULL, *operand_2 = NULL;
    expression_tree_node *node = NULL;
    stack_item *p_for_stack;

    err = stack_init(&st, sizeof(expression_tree_node *), free);
    if (err) {
        log_error("Error during stack initialization");
        return err;
    }

    token = string_init();
    if (token == NULL) {
        log_error("Failed to allocate memory for token");
        stack_free(st);
        return MEMORY_ALLOCATION_ERROR;
    }

    for (i = 0; i < string_len(postfix_exp); ++i) {
        pe = postfix_exp[i];

        if (pe == ' ') {
            if (string_len(token) == 0) {
                continue;
            }

            op = NULL;
            err = hash_table_get(operators, &token, (void **)&op);
            if (err != EXIT_SUCCESS && err != KEY_NOT_FOUND) {
                log_error("Error while getting element from hash table");
                string_free(token);
                stack_free(st);
                return err;
            }

            if (err == KEY_NOT_FOUND) {  // Operand
                node = (expression_tree_node *)malloc(
                    sizeof(expression_tree_node));
                if (node == NULL) {
                    log_error("Memory allocation error for operand node");
                    string_free(token);
                    stack_free(st);
                    return MEMORY_ALLOCATION_ERROR;
                }
                node->token = token;
                node->left = NULL;
                node->right = NULL;
                token = NULL;

                err = stack_push(st, &node);
                if (err) {
                    log_error("Error pushing operand to stack");
                    free(node);
                    string_free(token);
                    stack_free(st);
                    return err;
                }
            } else {  // Operator
                if (op->type == binary) {
                    stack_top(st, &p_for_stack);
                    operand_1 = *(expression_tree_node **)p_for_stack->data;
                    stack_pop(st);

                    stack_top(st, &p_for_stack);
                    operand_2 = *(expression_tree_node **)p_for_stack->data;
                    stack_pop(st);

                    node = (expression_tree_node *)malloc(
                        sizeof(expression_tree_node));
                    if (node == NULL) {
                        log_error("Memory allocation error for operator node");
                        string_free(token);
                        stack_free(st);
                        return MEMORY_ALLOCATION_ERROR;
                    }
                    node->token = token;
                    node->left = operand_2;
                    node->right = operand_1;
                    token = NULL;

                    err = stack_push(st, &node);
                    if (err) {
                        log_error("Error pushing operator node to stack");
                        free(node);
                        string_free(token);
                        stack_free(st);
                        return err;
                    }
                }
            }

            string_free(token);
            token = string_init();
            if (token == NULL) {
                log_error("Failed to allocate memory for token");
                stack_free(st);
                return MEMORY_ALLOCATION_ERROR;
            }
        } else {
            // Add character to token
            err = string_add(&token, pe);
            if (err) {
                log_error("Error adding character to token");
                string_free(token);
                stack_free(st);
                return err;
            }
        }
    }

    stack_top(st, &p_for_stack);

    operand_1 = *(expression_tree_node **)p_for_stack->data;

    stack_pop(st);

    *t = operand_1;

    string_free(token);
    stack_free(st);

    return EXIT_SUCCESS;
}

void __expression_tree_print_inner(expression_tree_node *t, size_t depth,
                                   const char *prefix) {
    if (t == NULL) {
        return;
    }

    char new_prefix[256];
    snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, "|    ");

    __expression_tree_print_inner(t->right, depth + 1, new_prefix);

    printf("%s", prefix);
    if (depth > 1) {
        printf("|-- ");
    }
    printf("'");
    string_print(t->token);
    printf("'");
    printf("\n");

    __expression_tree_print_inner(t->left, depth + 1, new_prefix);
}

void expression_tree_print(expression_tree *t) {
    if (t == NULL) {
        return;
    }
    __expression_tree_print_inner(t, 1, "");
}
