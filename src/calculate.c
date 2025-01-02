#include "calculate.h"

#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "../libc/cstring.h"
#include "../libc/hash_table.h"
#include "../libc/logger.h"
#include "cli.h"
#include "expression_tree.h"
#include "postfix_notation.h"

void calculate_operators_bucket_free(void *b) {
    hash_table_bucket *bucket = b;
    string_free(*(String *)bucket->key);
    free(bucket->key);
    free(bucket->value);
    free(bucket);
}

int calculate_operators_keys_compare(const void *a, const void *b) {
    String s1 = *(String *)a;
    String s2 = *(String *)b;
    return string_cmp(s1, s2);
}

void calculate_operands_bucket_free(void *b) {
    hash_table_bucket *bucket = b;
    string_free(*(String *)bucket->key);
    free(bucket->key);
    free(bucket->value);
    free(bucket);
}

int calculate_operands_keys_compare(const void *a, const void *b) {
    String s1 = *(String *)a;
    String s2 = *(String *)b;
    return string_cmp(s1, s2);
}

int calculate_add(int first_arg, ...) {
    va_list args;
    va_start(args, first_arg);
    int second_arg = va_arg(args, int);
    va_end(args);
    return first_arg + second_arg;
}

int calculate_sub(int first_arg, ...) {
    va_list args;
    va_start(args, first_arg);
    int second_arg = va_arg(args, int);
    va_end(args);
    return first_arg - second_arg;
}

int calculate_mul(int first_arg, ...) {
    va_list args;
    va_start(args, first_arg);
    int second_arg = va_arg(args, int);
    va_end(args);
    return first_arg * second_arg;
}

int calculate_div(int first_arg, ...) {
    va_list args;
    va_start(args, first_arg);
    int second_arg = va_arg(args, int);
    va_end(args);
    if (second_arg == 0) {
        log_error("Division by zero");
        return 0;
    }
    return first_arg / second_arg;
}

int calculate_mod(int first_arg, ...) {
    va_list args;
    va_start(args, first_arg);
    int second_arg = va_arg(args, int);
    va_end(args);
    if (second_arg == 0) {
        log_error("Modulus by zero");
        return 0;
    }
    return first_arg % second_arg;
}

int calculate_pow(int first_arg, ...) {
    va_list args;
    va_start(args, first_arg);
    int second_arg = va_arg(args, int);
    va_end(args);
    if (second_arg < 0) {
        log_warn("Negative exponent not supported");
        return 0;
    }

    int result = 1;
    while (second_arg > 0) {
        if (second_arg % 2 == 1) {
            result *= first_arg;
        }
        first_arg *= first_arg;
        second_arg /= 2;
    }
    return result;
}

// Unary Minus
int calculate_unary_minus(int first_arg, ...) { return -first_arg; }

// int calculate_is_op_binary(const String op) {
//     const char *valid_binary_operators[] = {"+", "*", "/", "%", "^"};
//     size_t num_operators =
//         sizeof(valid_binary_operators) / sizeof(valid_binary_operators[0]);
//     size_t i = 0;
//
//     if (strncmp(op, "~", 1) == 0) {
//         return 0;  // is unary
//     }
//
//     for (i = 0; i < num_operators; i++) {
//         if (strncmp(op, valid_binary_operators[i],
//                     strlen(valid_binary_operators[i])) == 0) {
//             return 1;
//         }
//     }
//     return -1;  // is not an operator
// }

int calculate_priorities(const String operator) {
    if (string_cmp_c(operator, "+") == 0 || string_cmp_c(operator, "-") == 0) {
        return 0;
    } else if (string_cmp_c(operator, "*") == 0 ||
               string_cmp_c(operator, "/") == 0 ||
               string_cmp_c(operator, "%") == 0) {
        return 1;
    } else if (string_cmp_c(operator, "~") == 0) {
        return 2;
    } else if (string_cmp_c(operator, "^") == 0) {
        return 3;
    } else if (string_cmp_c(operator, "(") == 0) {
        return INT_MIN;
    } else {
        return -1;  // Invalid operator
    }
}

int calculate_is_operator(const char *op) {
    const char *valid_operators[] = {"+", "-", "*", "/", "%", "~", "^"};
    size_t num_operators = sizeof(valid_operators) / sizeof(valid_operators[0]);
    size_t i = 0;

    for (i = 0; i < num_operators; i++) {
        if (strncmp(op, valid_operators[i], strlen(valid_operators[i])) == 0) {
            return strlen(valid_operators[i]);
        }
    }
    return 0;
}

err_t process_calculate_file(file_to_process *file) {
    if (file == NULL) {
        log_error("fin ptr is NULL");
        return DEREFERENCING_NULL_PTR;
    }

    char line[BUFSIZ];
    err_t err = 0;
    size_t len = 0, current_line = 0;
    FILE *fout = NULL;
    char error_filename[BUFSIZ];
    hash_table *operators = NULL, *operands = NULL;

    err = hash_table_init(&operators, calculate_operators_keys_compare,
                          djb2_hash, sizeof(String *), sizeof(operator_t),
                          calculate_operators_bucket_free);
    if (err) {
        log_error("error while initializing hash table");
        return err;
    }
    err = hash_table_init(&operands, calculate_operands_keys_compare, djb2_hash,
                          sizeof(String *), sizeof(int),
                          calculate_operands_bucket_free);
    if (err) {
        log_error("error while initializing hash table");
        hash_table_free(operators);
        return err;
    }

    err = calculate_fill_hash_table_with_operators(operators);
    if (err) {
        hash_table_free(operators);
        hash_table_free(operands);
        return err;
    }

    while (fgets(line, sizeof(line), file->data)) {
        len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';  // delete '\n' symbol
        }
        if (line[0] == '\0') {
            continue;
        }
        printf("Processing %zu line in %s file: \n", current_line,
               file->filename);
        err = process_calculate_line(line, operators, operands);
        if (err != EXIT_SUCCESS && err != INVALID_BRACES &&
            err != INVALID_SYMBOL && err != INVALID_OPERATIONS) {
            if (fout != NULL) {
                fclose(fout);
            }
            hash_table_free(operators);
            hash_table_free(operands);
            return err;
        }
        if (err == INVALID_BRACES) {
            if (fout == NULL) {
                sprintf(error_filename, "%s.errors", file->filename);
                fout = fopen(error_filename, "w");
                if (fout == NULL) {
                    log_error("Error while openning file for errors");

                    hash_table_free(operators);
                    hash_table_free(operands);
                    return OPENING_THE_FILE_ERROR;
                }
            }
            fprintf(fout, "%s : %zu : [%s] - Invalid braces placement error.\n",
                    file->filename, current_line, line);
            printf("Error occured. Skipping...\n");
            current_line++;
            continue;
        } else if (err == INVALID_SYMBOL) {
            if (fout == NULL) {
                sprintf(error_filename, "%s.errors", file->filename);
                fout = fopen(error_filename, "w");
                if (fout == NULL) {
                    log_error("Error while openning file for errors");
                    hash_table_free(operators);
                    hash_table_free(operands);
                    return OPENING_THE_FILE_ERROR;
                }
            }
            fprintf(fout, "%s : %zu : [%s] - Invalid symbol occurence error.\n",
                    file->filename, current_line, line);
            printf("Error occured. Skipping...\n");
            current_line++;
            continue;
        } else if (err == INVALID_OPERATIONS) {
            if (fout == NULL) {
                sprintf(error_filename, "%s.errors", file->filename);
                fout = fopen(error_filename, "w");
                if (fout == NULL) {
                    log_error("Error while openning file for errors");
                    hash_table_free(operators);
                    hash_table_free(operands);
                    return OPENING_THE_FILE_ERROR;
                }
            }
            fprintf(fout,
                    "%s : %zu : [%s] - Invalid operations and operands "
                    "combination.\n",
                    file->filename, current_line, line);
            printf("Error occured. Skipping...\n");
            current_line++;
            continue;
        }

        printf("Ok.\n\n");
        current_line++;
    }

    if (fout != NULL) {
        fclose(fout);
    }

    hash_table_free(operators);
    hash_table_free(operands);

    return EXIT_SUCCESS;
}

err_t process_calculate_line(char *line, hash_table *operators,
                             hash_table *operands) {
    if (line == NULL) {
        log_error("passed ptr is NULL");
        return DEREFERENCING_NULL_PTR;
    }

    err_t err = 0;
    String infix = NULL, postfix = NULL;
    expression_tree *tree = NULL;
    int res = 0;

    infix = string_from(line);
    if (infix == NULL) {
        log_error("Failed to allocate memory for infix string");
        return MEMORY_ALLOCATION_ERROR;
    }
    postfix = string_init();
    if (postfix == NULL) {
        log_error("Failed to allocate memory for postfix string");
        string_free(infix);
        return MEMORY_ALLOCATION_ERROR;
    }

    err = calculate_infix_to_postfix(infix, &postfix);
    if (err) {
        string_free(infix);
        string_free(postfix);
        return err;
    }

    printf("Source: (inf) ");
    string_print(infix);
    printf("\nConverted: (post) ");
    string_print(postfix);
    printf("\n\n");

    err = calculate_postfix_expression(postfix, &res, operators, operands);
    if (err) {
        string_free(infix);
        string_free(postfix);
        return err;
    }

    printf("Expression evalutation result: %d\n", res);

    // TODO: build arithmetic tree, calculate exp
    // err = expression_tree_fill_with_data_from_postfix_expression(
    //     &tree, postfix, calculate_is_op_binary);
    //
    // expression_tree_print(tree);
    //
    // expression_tree_free(tree);

    string_free(infix);
    string_free(postfix);

    return EXIT_SUCCESS;
}

err_t calculate_infix_to_postfix(const String infix_exp, String *postfix_exp) {
    return infix_to_postfix(infix_exp, isalnum, calculate_is_operator,
                            calculate_priorities, postfix_exp);
}

err_t calculate_fill_hash_table_with_operators(hash_table *operators) {
    if (operators == NULL) {
        log_error("Passed ptr is NULL");
        return DEREFERENCING_NULL_PTR;
    }

    operator_t *op = NULL;
    String representation = NULL;
    err_t err = 0;

    // Subtraction
    op = (operator_t *)malloc(sizeof(operator_t));
    if (op == NULL) {
        log_error("Memory allocation for subtraction operator error");
        return MEMORY_ALLOCATION_ERROR;
    }
    op->type = binary;
    representation = string_from("-");
    if (representation == NULL) {
        log_error(
            "Failed to allocate memory for subtraction operator "
            "representation");
        free(op);
        return MEMORY_ALLOCATION_ERROR;
    }
    op->func = calculate_sub;
    err = hash_table_set(operators, &representation, op);
    if (err) {
        free(op);
        string_free(representation);
        log_error("Failed to set subtraction operator to hash table");
        return err;
    }
    free(op);
    op = NULL;
    representation = NULL;

    // Multiplication
    op = (operator_t *)malloc(sizeof(operator_t));
    if (op == NULL) {
        log_error("Memory allocation for multiplication operator error");
        return MEMORY_ALLOCATION_ERROR;
    }
    op->type = binary;
    representation = string_from("*");
    if (representation == NULL) {
        log_error(
            "Failed to allocate memory for multiplication operator "
            "representation");
        free(op);
        return MEMORY_ALLOCATION_ERROR;
    }
    op->func = calculate_mul;
    err = hash_table_set(operators, &representation, op);
    if (err) {
        free(op);
        string_free(representation);
        log_error("Failed to set multiplication operator to hash table");
        return err;
    }
    free(op);
    op = NULL;
    representation = NULL;

    // Integer Division
    op = (operator_t *)malloc(sizeof(operator_t));
    if (op == NULL) {
        log_error("Memory allocation for division operator error");
        return MEMORY_ALLOCATION_ERROR;
    }
    op->type = binary;
    representation = string_from("/");
    if (representation == NULL) {
        log_error(
            "Failed to allocate memory for division operator representation");
        free(op);
        return MEMORY_ALLOCATION_ERROR;
    }
    op->func = calculate_div;
    err = hash_table_set(operators, &representation, op);
    if (err) {
        free(op);
        string_free(representation);
        log_error("Failed to set division operator to hash table");
        return err;
    }
    free(op);
    op = NULL;
    representation = NULL;

    // Modulus
    op = (operator_t *)malloc(sizeof(operator_t));
    if (op == NULL) {
        log_error("Memory allocation for modulus operator error");
        return MEMORY_ALLOCATION_ERROR;
    }
    op->type = binary;
    representation = string_from("%");
    if (representation == NULL) {
        log_error(
            "Failed to allocate memory for modulus operator representation");
        free(op);
        return MEMORY_ALLOCATION_ERROR;
    }
    op->func = calculate_mod;
    err = hash_table_set(operators, &representation, op);
    if (err) {
        free(op);
        string_free(representation);
        log_error("Failed to set modulus operator to hash table");
        return err;
    }
    free(op);
    op = NULL;
    representation = NULL;

    // Power
    op = (operator_t *)malloc(sizeof(operator_t));
    if (op == NULL) {
        log_error("Memory allocation for power operator error");
        return MEMORY_ALLOCATION_ERROR;
    }
    op->type = binary;
    representation = string_from("^");
    if (representation == NULL) {
        log_error(
            "Failed to allocate memory for power operator representation");
        free(op);
        return MEMORY_ALLOCATION_ERROR;
    }
    op->func = calculate_pow;
    err = hash_table_set(operators, &representation, op);
    if (err) {
        free(op);
        string_free(representation);
        log_error("Failed to set power operator to hash table");
        return err;
    }
    free(op);
    op = NULL;
    representation = NULL;

    // Unary minus
    op = (operator_t *)malloc(sizeof(operator_t));
    if (op == NULL) {
        log_error("Memory allocation for power operator error");
        return MEMORY_ALLOCATION_ERROR;
    }
    op->type = unary;
    representation = string_from("~");
    if (representation == NULL) {
        log_error(
            "Failed to allocate memory for power operator representation");
        free(op);
        return MEMORY_ALLOCATION_ERROR;
    }
    op->func = calculate_unary_minus;
    err = hash_table_set(operators, &representation, op);
    if (err) {
        free(op);
        string_free(representation);
        log_error("Failed to set power operator to hash table");
        return err;
    }
    free(op);
    op = NULL;
    representation = NULL;

    // Addition
    op = (operator_t *)malloc(sizeof(operator_t));
    if (op == NULL) {
        log_error("Memory allocation for addition operator error");
        return MEMORY_ALLOCATION_ERROR;
    }
    op->type = binary;
    representation = string_from("+");
    if (representation == NULL) {
        log_error(
            "Failed to allocate memory for addition operator representation");
        free(op);
        return MEMORY_ALLOCATION_ERROR;
    }
    op->func = calculate_add;
    err = hash_table_set(operators, &representation, op);
    if (err) {
        free(op);
        string_free(representation);
        log_error("Failed to set addition operator to hash table");
        return err;
    }
    free(op);
    op = NULL;
    representation = NULL;

    return EXIT_SUCCESS;
}
