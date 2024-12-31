#include "postfix_notation.h"

#include <ctype.h>
#include <time.h>

#include "../libc/logger.h"
#include "../libc/stack.h"
#include "../libc/types.h"

int operation_comparer(void const *first, void const *second) {
    return (*((operation **)first))->representation -
           (*((operation **)second))->representation;
}

err_t infix_to_postfix(const String infix_exp, int (*is_operand)(int c),
                       int (*is_operator)(int c), int (*priority_mapper)(int c),
                       String *postfix_exp) {
    if (infix_exp == NULL || is_operand == NULL || is_operator == NULL ||
        priority_mapper == NULL || postfix_exp == NULL) {
        log_error("passed NULL ptr");
        return DEREFERENCING_NULL_PTR;
    }

    char ie, prev = ' ';
    stack *operators = NULL;
    char for_stack = 0;
    stack_item *p_for_stack = NULL;
    err_t err = 0;
    size_t i = 0;

    err = stack_init(&operators, sizeof(char), free);
    if (err) {
        log_error("failed to initialize stack");
        return err;
    }

    for_stack = '(';
    err = stack_push(operators, &for_stack);
    if (err) {
        log_error("failed to push to stack");
        stack_free(operators);
        return err;
    }

    for (i = 0; i < string_len(infix_exp); ++i) {
        ie = infix_exp[i];

        log_trace("ie: %c", ie);

        if (is_operand(ie)) {
            err = string_add(postfix_exp, ie);
            if (err) {
                log_error("failed to push to the string");
                stack_free(operators);
                return err;
            }
        } else {
            if (is_operand(prev)) {
                err = string_add(postfix_exp, ' ');
                if (err) {
                    log_error("failed to push to the string");
                    stack_free(operators);
                    return err;
                }
            }
            if (ie == '(') {
                for_stack = ie;
                err = stack_push(operators, &for_stack);
                if (err) {
                    log_error("failed to push to stack");
                    stack_free(operators);
                    return err;
                }
            } else if (ie == ')') {  // or \0 mb
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
                    for_stack = *(char *)p_for_stack->data;
                    err = stack_pop(operators);
                    if (err) {
                        log_error("failed pop from stack");
                        stack_free(operators);
                        return err;
                    }

                    if (for_stack == '(') {
                        break;
                    }
                    err = string_add(postfix_exp, for_stack);
                    if (err) {
                        log_error("failed to push to the string");
                        stack_free(operators);
                        return err;
                    }
                    err = string_add(postfix_exp, ' ');
                    if (err) {
                        log_error("failed to push to the string");
                        stack_free(operators);
                        return err;
                    }
                }
            } else if (is_operator(ie)) {
                log_debug("found operator");
                while (1) {
                    err = stack_top(operators, &p_for_stack);
                    if (err != EXIT_SUCCESS && err != STACK_IS_EMPTY) {
                        log_error("top from stack failed: %d", err);
                        stack_free(operators);
                        return err;
                    }

                    if (err == STACK_IS_EMPTY) {
                        log_error("stack_is_empty");
                        stack_free(operators);
                        return INVALID_BRACES;
                    }

                    for_stack = *(char *)p_for_stack->data;
                    log_debug("comparison %c %c", for_stack, ie);
                    if (priority_mapper(for_stack) < priority_mapper(ie)) {
                        log_debug("first < second, break");
                        break;
                    }
                    log_debug("first > second, push");

                    err = stack_pop(operators);
                    if (err != EXIT_SUCCESS && err != STACK_IS_EMPTY) {
                        log_error("pop from stack failed");
                        stack_free(operators);
                        return err;
                    }
                    if (err == STACK_IS_EMPTY) {
                        log_error("stack is empty");
                        stack_free(operators);
                        return INVALID_BRACES;
                    }

                    err = string_add(postfix_exp, for_stack);
                    if (err) {
                        log_error("failed to push to the string");
                        stack_free(operators);
                        return err;
                    }
                    err = string_add(postfix_exp, ' ');
                    if (err) {
                        log_error("failed to push to the string");
                        stack_free(operators);
                        return err;
                    }
                }
                for_stack = ie;
                err = stack_push(operators, &ie);
                if (err) {
                    log_error("failed push to stack");
                    stack_free(operators);
                    return err;
                }
            } else if (ie != ' ' && !is_operand(ie)) {
                log_error("invalid symbol found: %c", ie);
            }
        }
        prev = ie;
    }

    if (is_operand(prev) || is_operator(prev)) {
        err = string_add(postfix_exp, ' ');
        if (err) {
            log_error("failed to push to the string");
            stack_free(operators);
            return err;
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
            log_error("stack is empty");
            return INVALID_BRACES;
        }
        for_stack = *(char *)p_for_stack->data;
        err = stack_pop(operators);
        if (err) {
            log_error("failed pop from stack");
            stack_free(operators);
            return err;
        }

        if (for_stack == '(') {
            break;
        }
        err = string_add(postfix_exp, for_stack);
        if (err) {
            log_error("failed to push to the string");
            stack_free(operators);
            return err;
        }
        err = string_add(postfix_exp, ' ');
        if (err) {
            log_error("failed to push to the string");
            stack_free(operators);
            return err;
        }
    }

    if (!stack_is_empty(operators)) {
        log_error("transformation done, stack is not emtpy, invalid braces");
        stack_free(operators);
        return INVALID_BRACES;
    }

    stack_free(operators);

    return EXIT_SUCCESS;
}

err_t calculate_postfix_expression(const String postfix_exp,
                                   int *expression_result,
                                   size_t operations_count, ...) {
    if (postfix_exp == NULL || expression_result == NULL) {
        log_error("Passed ptr is NULL");
        return DEREFERENCING_NULL_PTR;
    }

    operation **operations = NULL, current_operation,
              *current_operation_ptr = &current_operation,
              *found_operation_ptr = NULL;
    stack *operands = NULL;
    err_t err = 0;
    va_list ap;
    size_t i = 0;
    char pe = 0, buf[BUFSIZ], *b = buf, prev = ' ';
    int right_operand, left_operand;
    stack_item *p_for_stack = NULL;
    int for_stack;

    operations = (operation **)malloc(sizeof(operation *) * operations_count);
    if (operations == NULL) {
        log_error("Failed allocate memory for operations");
        return MEMORY_ALLOCATION_ERROR;
    }

    err = stack_init(&operands, sizeof(int), free);
    if (err) {
        log_error("failed to initialize stack");
        free(operations);
        return err;
    }

    va_start(ap, operations_count);
    for (i = 0; i < operations_count; ++i) {
        operations[i] = va_arg(ap, operation *);
        if (operations[i] == NULL) {
            log_error("failed to get operations from va_list");
            free(operations);
            stack_free(operands);
            return INVALID_INPUT_DATA;
        }
    }
    va_end(ap);

    qsort(operations, operations_count, sizeof(operation *),
          operation_comparer);

    for (i = 0; i < string_len(postfix_exp); ++i) {
        pe = postfix_exp[i];
        if (isdigit(pe)) {
            *b++ = pe;
        } else {
            if (isdigit(prev)) {
                *b = 0;
                b = buf;
                err = catoi(buf, 10, &right_operand);
                if (err) {
                    log_error("Failed to convert buf to integer");
                    free(operations);
                    stack_free(operands);
                    return err;
                }
                err = stack_push(operands, &right_operand);
                if (err) {
                    log_error("Failed to push to stack");
                    free(operations);
                    stack_free(operands);
                    return err;
                }
            }
            // TODO: redo
            // if (binary_search(operations, &current_operation_ptr,
            //                   operations_count, sizeof(operation *),
            //                   operation_comparer,
            //                   (void **)&found_operation_ptr) ==
            //     NO_SUCH_ENTRY_IN_COLLECTION) {
            //     if (i == string_len(postfix_exp - 1)) {
            //         err = stack_top(operands, &p_for_stack);
            //         if (err) {
            //             log_error("Failed to top from stack");
            //             free(operations);
            //             stack_free(operands);
            //             return err;
            //         }
            //         break;
            //     }
        }
    }

    return EXIT_SUCCESS;
}
