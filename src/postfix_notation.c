#include "postfix_notation.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../libc/logger.h"
#include "../libc/stack.h"
#include "../libc/types.h"
#include "../libc/utils.h"

void postfix_notation_string_free(void *s) {
    String *st = s;
    string_free(*st);
    free(st);
    return;
}

err_t infix_to_postfix(const String infix_exp, int (*is_operand)(int c),
                       int (*is_operator)(const char *op),
                       int (*priority_mapper)(const String op),
                       String *postfix_exp) {
    if (infix_exp == NULL || is_operand == NULL || is_operator == NULL ||
        priority_mapper == NULL || postfix_exp == NULL) {
        log_error("passed NULL ptr");
        return DEREFERENCING_NULL_PTR;
    }

    stack *operators = NULL;
    stack_item *p_for_stack = NULL;
    err_t err = 0;
    size_t i = 0, j = 0;
    char ie = 0;
    char *current_p = NULL;
    String buffer = NULL,
           for_stack = NULL;  // Buffer for multi-character operators
    size_t buffer_len = 0, exp_len = string_len(infix_exp);

    err =
        stack_init(&operators, sizeof(String *), postfix_notation_string_free);
    if (err) {
        log_error("failed to initialize stack");
        return err;
    }

    buffer = string_from("(");
    if (buffer == NULL) {
        stack_free(operators);
        log_error("Failed to allocate memory for buffer");
        return MEMORY_ALLOCATION_ERROR;
    }
    err = stack_push(operators, &buffer);
    if (err) {
        log_error("failed to push to stack");
        string_free(buffer);
        stack_free(operators);
        return err;
    }
    buffer = NULL;

    for (i = 0; i < exp_len; ++i) {
        current_p = infix_exp + i;
        ie = infix_exp[i];

        buffer_len = is_operator(current_p);
        if (buffer_len > 0) {  // found operator

            buffer = string_init();
            if (buffer == NULL) {
                stack_free(operators);
                log_error("Failed to allocate memory for buffer");
                return MEMORY_ALLOCATION_ERROR;
            }
            for (j = 0; j < buffer_len; ++j) {
                err = string_add(&buffer, current_p[j]);
                if (err) {
                    log_error("failed push to string");
                    stack_free(operators);
                    string_free(buffer);
                    return err;
                }
                i++;
            }

            i--;
            ie = infix_exp[i];
            current_p = infix_exp + i;

            while (1) {
                err = stack_top(operators, &p_for_stack);
                if (err != EXIT_SUCCESS && err != STACK_IS_EMPTY) {
                    log_error("top from stack failed: %d", err);
                    stack_free(operators);
                    string_free(buffer);
                    return err;
                }

                if (err == STACK_IS_EMPTY) {
                    log_error("stack_is_empty, invalid braces placement");
                    stack_free(operators);
                    string_free(buffer);
                    return INVALID_BRACES;
                }

                for_stack = *(String *)p_for_stack->data;
                if (priority_mapper(for_stack) < priority_mapper(buffer)) {
                    break;
                }

                err = string_cat(postfix_exp, &for_stack);
                if (err) {
                    log_error("failed to cat to the string");
                    stack_free(operators);
                    string_free(buffer);
                    return err;
                }

                err = string_add(postfix_exp, ' ');
                if (err) {
                    log_error("failed to push to the string");
                    stack_free(operators);
                    string_free(buffer);
                    return err;
                }
                err = stack_pop(operators);
                if (err != EXIT_SUCCESS && err != STACK_IS_EMPTY) {
                    log_error("pop from stack failed");
                    stack_free(operators);
                    return err;
                }
                for_stack = NULL;
                if (err == STACK_IS_EMPTY) {
                    log_error("stack is empty");
                    stack_free(operators);
                    return INVALID_BRACES;
                }
            }
            err = stack_push(operators, &buffer);
            if (err) {
                log_error("failed push to stack");
                stack_free(operators);
                string_free(buffer);
                return err;
            }

            buffer = NULL;
            continue;
        } else if (ie == '(') {
            buffer = string_from("(");
            if (buffer == NULL) {
                stack_free(operators);
                log_error("Failed to allocate memory for buffer");
                return MEMORY_ALLOCATION_ERROR;
            }
            err = stack_push(operators, &buffer);
            if (err) {
                log_error("failed to push to stack");
                string_free(buffer);
                stack_free(operators);
                return err;
            }
            buffer = NULL;

        } else if (ie == ')') {
            while (1) {
                err = stack_top(operators, &p_for_stack);
                if (err != EXIT_SUCCESS && err != STACK_IS_EMPTY) {
                    log_error("top from stack failed");
                    stack_free(operators);
                    return err;
                }
                if (err == STACK_IS_EMPTY) {
                    log_error("stack_is_empty");
                    stack_free(operators);
                    return INVALID_BRACES;
                }
                buffer = *(String *)p_for_stack->data;

                if (buffer[0] == '(') {
                    err = stack_pop(operators);
                    if (err) {
                        log_error("failed pop from stack");
                        stack_free(operators);
                        return err;
                    }
                    break;
                }
                err = string_cat(postfix_exp, &buffer);
                if (err) {
                    log_error("failed to cat to the string");
                    stack_free(operators);
                    return err;
                }
                err = string_add(postfix_exp, ' ');
                if (err) {
                    log_error("failed to push to the string");
                    stack_free(operators);
                    return err;
                }

                err = stack_pop(operators);
                if (err) {
                    log_error("failed pop from stack");
                    stack_free(operators);
                    return err;
                }
            }
        } else if (is_operand(ie)) {
            while (is_operand(ie) && i < exp_len) {
                err = string_add(postfix_exp, ie);
                if (err) {
                    log_error("failed to push to the string");
                    stack_free(operators);
                    return err;
                }
                i++;
                ie = infix_exp[i];
            }
            string_add(postfix_exp, ' ');
            if (err) {
                log_error("failed to push to the string");
                stack_free(operators);
                return err;
            }
            i--;
            ie = infix_exp[i];
            continue;

        } else if (ie != ' ') {
            log_error("invalid symbol '%c' found", ie);
            stack_free(operators);
            return INVALID_SYMBOL;
        }
    }

    while (1) {
        err = stack_top(operators, &p_for_stack);
        if (err != EXIT_SUCCESS && err != STACK_IS_EMPTY) {
            log_error("top from stack failed");
            stack_free(operators);
            return err;
        }
        if (err == STACK_IS_EMPTY) {
            log_error("stack is empty, invalid braces placement");
            stack_free(operators);
            return INVALID_BRACES;
        }
        buffer = *(String *)p_for_stack->data;

        if (buffer[0] == '(') {
            err = stack_pop(operators);
            if (err) {
                log_error("failed pop from stack");
                stack_free(operators);
                return err;
            }
            break;
        }
        err = string_cat(postfix_exp, &buffer);
        if (err) {
            log_error("failed to cat to the string");
            stack_free(operators);
            return err;
        }
        err = string_add(postfix_exp, ' ');
        if (err) {
            log_error("failed to cat to the string");
            stack_free(operators);
            return err;
        }
        err = stack_pop(operators);
        if (err) {
            log_error("failed pop from stack");
            stack_free(operators);
            return err;
        }
    }

    if (!stack_is_empty(operators)) {
        log_error(
            "transformation done, stack is not emtpy, invalid braces, %zu",
            operators->size);
        stack_free(operators);
        return INVALID_BRACES;
    }

    exp_len = string_len(*postfix_exp);

    if (exp_len > 0 && (*postfix_exp)[exp_len - 1] == ' ') {
        string_grow(postfix_exp, exp_len);
    }

    stack_free(operators);
    return EXIT_SUCCESS;
}

err_t calculate_postfix_expression(const String postfix_exp,
                                   int *expression_result,
                                   hash_table *operators,
                                   hash_table *operands) {
    if (postfix_exp == NULL || expression_result == NULL || operators == NULL ||
        operands == NULL) {
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
                if (isdigit_s(token)) {  // found token is digits, call gorner
                    err = catoi_s(token, 10, &operand_1);  // and push to stack
                    if (err) {
                        log_error("failed to transmit str to int");
                        stack_free(st);
                        string_free(token);
                        return err;
                    }
                    err = stack_push(st, &operand_1);
                    if (err) {
                        log_error("failed push to stack");
                        stack_free(st);
                        string_free(token);
                        return err;
                    }
                    operand_1 = 0;
                } else {  // token is varialbe, check if it in hash table
                    err = hash_table_get(operands, &token,
                                         (void **)&get_from_hash_table);
                    if (err != EXIT_SUCCESS && err != KEY_NOT_FOUND) {
                        log_error("Error while getting elem from hash table");
                        stack_free(st);
                        string_free(token);
                        return err;
                    }
                    if (err == EXIT_SUCCESS) {
                        operand_1 = *get_from_hash_table;
                    }
                    if (err == KEY_NOT_FOUND) {  // variable not found in hash
                                                 // table, asking user for it

                        printf("Please enter value for '");
                        string_print(token);
                        printf("' variable: ");
                        while (1) {
                            if (scanf("%d", &operand_1) == 1) {
                                break;
                            } else {
                                printf(
                                    "Invalid input. Please enter a valid "
                                    "integer.\n");

                                while (getchar() != '\n');
                                printf("Please try again: ");
                            }
                        }

                        for_hash_table = string_init();
                        if (for_hash_table == NULL) {
                            log_error("Error to allocate memory for string");
                            stack_free(st);
                            string_free(token);
                            return err;
                        }
                        err = string_cpy(&for_hash_table, &token);
                        if (err) {
                            log_error("Error cpy string");
                            stack_free(st);
                            string_free(token);
                            string_free(for_hash_table);
                            return err;
                        }

                        err = hash_table_set(operands, &for_hash_table,
                                             &operand_1);
                        if (err) {
                            log_error("Error push to hash table");
                            stack_free(st);
                            string_free(token);
                            return err;
                        }
                        for_hash_table = NULL;
                    }

                    // pushing variable to stack
                    err = stack_push(st, &operand_1);
                    if (err) {
                        log_error("Error push to stack");
                        stack_free(st);
                        string_free(token);
                        return err;
                    }
                }
            } else {  // operator found
                if (op->type == binary) {
                    err = stack_top(st, &p_for_stack);
                    if (err != EXIT_SUCCESS && err != STACK_IS_EMPTY) {
                        log_error("Error top from stack");
                        stack_free(st);
                        string_free(token);
                        return err;
                    }
                    if (err == STACK_IS_EMPTY) {
                        log_error(
                            "stack is empty, not enouth operands for "
                            "operators");
                        stack_free(st);
                        string_free(token);
                        return INVALID_OPERATIONS;
                    }
                    operand_1 = *(int *)p_for_stack->data;
                    err = stack_pop(st);
                    if (err) {
                        log_error("Error pop from stack");
                        stack_free(st);
                        string_free(token);
                        return err;
                    }

                    err = stack_top(st, &p_for_stack);
                    if (err != EXIT_SUCCESS && err != STACK_IS_EMPTY) {
                        log_error("Error top from stack");
                        stack_free(st);
                        string_free(token);
                        return err;
                    }
                    if (err == STACK_IS_EMPTY) {
                        log_error(
                            "stack is empty, not enouth operands for "
                            "operators");
                        stack_free(st);
                        string_free(token);
                        return INVALID_OPERATIONS;
                    }
                    operand_2 = *(int *)p_for_stack->data;

                    err = stack_pop(st);
                    if (err) {
                        log_error("Error pop from stack");
                        stack_free(st);
                        string_free(token);
                        return err;
                    }

                    result = op->func(operand_2, operand_1);

                    err = stack_push(st, &result);
                    if (err) {
                        log_error("Error push to stack");
                        stack_free(st);
                        string_free(token);
                        return err;
                    }
                } else {  // op is unary
                    err = stack_top(st, &p_for_stack);
                    if (err != EXIT_SUCCESS && err != STACK_IS_EMPTY) {
                        log_error("Error top from stack");
                        stack_free(st);
                        string_free(token);
                        return err;
                    }
                    if (err == STACK_IS_EMPTY) {
                        log_error(
                            "stack is empty, not enouth operands for "
                            "operators");
                        stack_free(st);
                        string_free(token);
                        return INVALID_OPERATIONS;
                    }
                    operand_1 = *(int *)p_for_stack->data;
                    err = stack_pop(st);
                    if (err) {
                        log_error("Error pop from stack");
                        stack_free(st);
                        string_free(token);
                        return err;
                    }
                    result = op->func(operand_1);

                    err = stack_push(st, &result);
                    if (err) {
                        log_error("Error push to stack");
                        stack_free(st);
                        string_free(token);
                        return err;
                    }
                }
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

    if (stack_is_empty(st)) {
        *expression_result = 0;
        string_free(token);
        stack_free(st);
        return EXIT_SUCCESS;
    }
    err = stack_top(st, &p_for_stack);
    if (err) {  // stack is not empty
        log_error("failed top from stack");
        string_free(token);
        stack_free(st);
        return err;
    }
    result = *(int *)p_for_stack->data;
    err = stack_pop(st);
    if (err) {  // stack is not empty
        log_error("failed pop from stack");
        string_free(token);
        stack_free(st);
        return err;
    }

    if (!stack_is_empty(st)) {
        log_error(
            "evaluation ended, stack is not empty, invalid operators and "
            "operands combination");
        string_free(token);
        stack_free(st);
        return INVALID_OPERATIONS;
    }

    *expression_result = result;

    stack_free(st);
    string_free(token);

    return EXIT_SUCCESS;
}
