#include "postfix_notation.h"

#include <ctype.h>
#include <string.h>
#include <time.h>

#include "../libc/logger.h"
#include "../libc/stack.h"
#include "../libc/types.h"

int operation_comparer(void const *first, void const *second) {
    return (*((operation **)first))->representation -
           (*((operation **)second))->representation;
}

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
