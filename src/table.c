#include "table.h"

#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "../libc/logger.h"
#include "cli.h"
#include "postfix_notation.h"

void table_u_list_free(void *s) {
    String *st = s;
    string_free(*st);
    free(st);
    return;
}

err_t table_validate_postfix(const String postfix_exp, hash_table *operators) {
    if (postfix_exp == NULL || operators == NULL) {
        log_error("passed ptr in NULL");
        return DEREFERENCING_NULL_PTR;
    }

    err_t err = 0;
    size_t i = 0;
    String token = NULL;
    char pe = 0, to_validate = 0;
    operator_t *op = NULL;

    token = string_init();
    if (token == NULL) {
        log_error("failed to allocate memory for string");
        return MEMORY_ALLOCATION_ERROR;
    }

    for (i = 0; i < string_len(postfix_exp); ++i) {
        pe = postfix_exp[i];

        if (pe == ' ') {
            err = hash_table_get(operators, &token, (void **)&op);
            if (err != EXIT_SUCCESS && err != KEY_NOT_FOUND) {
                log_error("failed to get from hash table");
                string_free(token);
                return err;
            }
            if (err == KEY_NOT_FOUND) {  // is operand
                // log_debug("%s", token);
                if (string_len(token) >
                    1) {  // can be only one-letter vars and '1', '0'
                    log_error("invalid operator found");
                    string_free(token);
                    return INVALID_OPERAND;
                }
                to_validate = token[0];
                if (isdigit(to_validate) && to_validate != '0' &&
                    to_validate != '1') {
                    log_error("numeric operator can be only '0' or '1'");
                    string_free(token);
                    return INVALID_OPERAND;
                }
            }
            string_free(token);
            token = NULL;
            token = string_init();
            if (token == NULL) {
                log_error("memory allocation error");
                return MEMORY_ALLOCATION_ERROR;
            }
        } else {
            err = string_add(&token, pe);
            if (err) {
                log_error("string add failed");
                string_free(token);
                return err;
            }
        }
    }

    string_free(token);

    return EXIT_SUCCESS;
}

void table_operators_bucket_free(void *b) {
    hash_table_bucket *bucket = b;
    string_free(*(String *)bucket->key);
    free(bucket->key);
    free(bucket->value);
    free(bucket);
}

int table_operators_keys_compare(const void *a, const void *b) {
    String s1 = *(String *)a;
    String s2 = *(String *)b;
    return string_cmp(s1, s2);
}

void table_operands_bucket_free(void *b) {
    hash_table_bucket *bucket = b;
    string_free(*(String *)bucket->key);
    free(bucket->key);
    free(bucket->value);
    free(bucket);
}

int table_operands_keys_compare(const void *a, const void *b) {
    String s1 = *(String *)a;
    String s2 = *(String *)b;
    return string_cmp(s1, s2);
}

int table_and(int first_arg, ...) {
    va_list args;
    va_start(args, first_arg);
    int second_arg = va_arg(args, int);
    va_end(args);
    return first_arg & second_arg;
}

int table_or(int first_arg, ...) {
    va_list args;
    va_start(args, first_arg);
    int second_arg = va_arg(args, int);
    va_end(args);
    return first_arg | second_arg;
}

int table_implication(int first_arg, ...) {
    va_list args;
    va_start(args, first_arg);
    int second_arg = va_arg(args, int);
    va_end(args);
    return (~first_arg) | second_arg;
}

int table_coimplication(int first_arg, ...) {
    va_list args;
    va_start(args, first_arg);
    int second_arg = va_arg(args, int);
    va_end(args);
    return (first_arg & second_arg) | ((~first_arg) & (~second_arg));
}

int table_logical_addition(int first_arg, ...) {
    va_list args;
    va_start(args, first_arg);
    int second_arg = va_arg(args, int);
    va_end(args);
    return first_arg ^ second_arg;
}

int table_equivalence(int first_arg, ...) {
    va_list args;
    va_start(args, first_arg);
    int second_arg = va_arg(args, int);
    va_end(args);
    return !(first_arg ^ second_arg);
}

int table_not(int first_arg, ...) { return ~first_arg; }

int table_sheffer_stroke(int first_arg, ...) {
    va_list args;
    va_start(args, first_arg);
    int second_arg = va_arg(args, int);
    va_end(args);
    return ~(first_arg & second_arg);
}

int table_webber_function(int first_arg, ...) {
    va_list args;
    va_start(args, first_arg);
    int second_arg = va_arg(args, int);
    va_end(args);
    return first_arg ^ second_arg;
}

int table_priorities(const String operator) {
    if (string_cmp_c(operator, "&") == 0) {
        return 0;
    } else if (string_cmp_c(operator, "|") == 0) {
        return 1;
    } else if (string_cmp_c(operator, "~") == 0) {
        return 2;
    } else if (string_cmp_c(operator, "->") == 0) {
        return 3;
    } else if (string_cmp_c(operator, "+>") == 0) {
        return 4;
    } else if (string_cmp_c(operator, "<>") == 0) {
        return 5;
    } else if (string_cmp_c(operator, "=") == 0) {
        return 6;
    } else if (string_cmp_c(operator, "!") == 0) {
        return 7;
    } else if (string_cmp_c(operator, "?") == 0) {
        return 8;
    } else if (string_cmp_c(operator, "(") == 0) {
        return INT_MIN;
    } else {
        return -1;
    }
}

int table_is_operator(const char *op) {
    const char *valid_operators[] = {"&",  "|", "~", "->", "+>",
                                     "<>", "=", "!", "?"};
    size_t num_operators = sizeof(valid_operators) / sizeof(valid_operators[0]);
    size_t i = 0;

    for (i = 0; i < num_operators; i++) {
        if (strncmp(op, valid_operators[i], strlen(valid_operators[i])) == 0) {
            return strlen(valid_operators[i]);
        }
    }
    return 0;
}

err_t table_infix_to_postfix(const String infix_exp, String *postfix_exp) {
    return infix_to_postfix(infix_exp, isalnum, table_is_operator,
                            table_priorities, postfix_exp);
}

err_t process_table_file(file_to_process *file) {
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

    err = hash_table_init(&operators, table_operators_keys_compare, djb2_hash,
                          sizeof(String *), sizeof(operator_t),
                          table_operators_bucket_free);
    if (err) {
        log_error("error while initializing hash table");
        return err;
    }
    err = hash_table_init(&operands, table_operands_keys_compare, djb2_hash,
                          sizeof(String *), sizeof(int),
                          table_operands_bucket_free);
    if (err) {
        log_error("error while initializing hash table");
        hash_table_free(operators);
        return err;
    }

    err = table_fill_hash_table_with_operators(operators);
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
        printf("Processing %zu line in %s file: \n\n", current_line,
               file->filename);
        err = process_table_line(line, operators);
        if (err != EXIT_SUCCESS && err != INVALID_BRACES &&
            err != INVALID_SYMBOL && err != INVALID_OPERATIONS &&
            err != INVALID_OPERAND) {
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
            printf("Error occured. Skipping...\n\n");
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
            printf("Error occured. Skipping...\n\n");
            current_line++;
            continue;
        } else if (err == INVALID_OPERAND) {
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
            fprintf(fout, "%s : %zu : [%s] - Invalid operand format.\n",
                    file->filename, current_line, line);
            printf("Error occured. Skipping...\n\n");
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

err_t process_table_line(char *line, hash_table *operators) {
    if (line == NULL || operators == NULL) {
        log_error("passed ptr is NULL");
        return DEREFERENCING_NULL_PTR;
    }

    err_t err = 0;
    String infix = NULL, postfix = NULL;
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

    err = table_infix_to_postfix(infix, &postfix);
    if (err) {
        string_free(infix);
        string_free(postfix);
        return err;
    }

    err = table_validate_postfix(postfix, operators);
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

    err = table_create_table_of_truth(postfix, operators);
    if (err) {
        string_free(infix);
        string_free(postfix);
        return err;
    }

    string_free(infix);
    string_free(postfix);

    return EXIT_SUCCESS;
}

err_t table_fill_hash_table_with_operators(hash_table *operators) {
    if (operators == NULL) {
        log_error("Passed ptr is NULL");
        return DEREFERENCING_NULL_PTR;
    }

    operator_t *op = NULL;
    String representation = NULL;
    err_t err = 0;

    // Logical AND
    op = (operator_t *)malloc(sizeof(operator_t));
    if (op == NULL) {
        log_error("Memory allocation for AND operator error");
        return MEMORY_ALLOCATION_ERROR;
    }
    op->type = binary;
    representation = string_from("&");
    if (representation == NULL) {
        log_error("Failed to allocate memory for AND operator representation");
        free(op);
        return MEMORY_ALLOCATION_ERROR;
    }
    op->func = table_and;
    err = hash_table_set(operators, &representation, op);
    if (err) {
        free(op);
        string_free(representation);
        log_error("Failed to set AND operator to hash table");
        return err;
    }
    free(op);
    op = NULL;
    representation = NULL;

    // Logical OR
    op = (operator_t *)malloc(sizeof(operator_t));
    if (op == NULL) {
        log_error("Memory allocation for OR operator error");
        return MEMORY_ALLOCATION_ERROR;
    }
    op->type = binary;
    representation = string_from("|");
    if (representation == NULL) {
        log_error("Failed to allocate memory for OR operator representation");
        free(op);
        return MEMORY_ALLOCATION_ERROR;
    }
    op->func = table_or;
    err = hash_table_set(operators, &representation, op);
    if (err) {
        free(op);
        string_free(representation);
        log_error("Failed to set OR operator to hash table");
        return err;
    }
    free(op);
    op = NULL;
    representation = NULL;

    // Logical NOT
    op = (operator_t *)malloc(sizeof(operator_t));
    if (op == NULL) {
        log_error("Memory allocation for NOT operator error");
        return MEMORY_ALLOCATION_ERROR;
    }
    op->type = unary;
    representation = string_from("~");
    if (representation == NULL) {
        log_error("Failed to allocate memory for NOT operator representation");
        free(op);
        return MEMORY_ALLOCATION_ERROR;
    }
    op->func = table_not;
    err = hash_table_set(operators, &representation, op);
    if (err) {
        free(op);
        string_free(representation);
        log_error("Failed to set NOT operator to hash table");
        return err;
    }
    free(op);
    op = NULL;
    representation = NULL;

    // Logical Implication (->)
    op = (operator_t *)malloc(sizeof(operator_t));
    if (op == NULL) {
        log_error("Memory allocation for implication operator error");
        return MEMORY_ALLOCATION_ERROR;
    }
    op->type = binary;
    representation = string_from("->");
    if (representation == NULL) {
        log_error(
            "Failed to allocate memory for implication operator "
            "representation");
        free(op);
        return MEMORY_ALLOCATION_ERROR;
    }
    op->func = table_implication;
    err = hash_table_set(operators, &representation, op);
    if (err) {
        free(op);
        string_free(representation);
        log_error("Failed to set implication operator to hash table");
        return err;
    }
    free(op);
    op = NULL;
    representation = NULL;

    // Logical Coimplication (+>)
    op = (operator_t *)malloc(sizeof(operator_t));
    if (op == NULL) {
        log_error("Memory allocation for coimplication operator error");
        return MEMORY_ALLOCATION_ERROR;
    }
    op->type = binary;
    representation = string_from("+>");
    if (representation == NULL) {
        log_error(
            "Failed to allocate memory for coimplication operator "
            "representation");
        free(op);
        return MEMORY_ALLOCATION_ERROR;
    }
    op->func = table_coimplication;
    err = hash_table_set(operators, &representation, op);
    if (err) {
        free(op);
        string_free(representation);
        log_error("Failed to set coimplication operator to hash table");
        return err;
    }
    free(op);
    op = NULL;
    representation = NULL;

    // Logical Addition (<>)
    op = (operator_t *)malloc(sizeof(operator_t));
    if (op == NULL) {
        log_error("Memory allocation for logical addition operator error");
        return MEMORY_ALLOCATION_ERROR;
    }
    op->type = binary;
    representation = string_from("<>");
    if (representation == NULL) {
        log_error(
            "Failed to allocate memory for logical addition operator "
            "representation");
        free(op);
        return MEMORY_ALLOCATION_ERROR;
    }
    op->func = table_logical_addition;
    err = hash_table_set(operators, &representation, op);
    if (err) {
        free(op);
        string_free(representation);
        log_error("Failed to set logical addition operator to hash table");
        return err;
    }
    free(op);
    op = NULL;
    representation = NULL;

    // Logical Equivalence (=)
    op = (operator_t *)malloc(sizeof(operator_t));
    if (op == NULL) {
        log_error("Memory allocation for equivalence operator error");
        return MEMORY_ALLOCATION_ERROR;
    }
    op->type = binary;
    representation = string_from("=");
    if (representation == NULL) {
        log_error(
            "Failed to allocate memory for equivalence operator "
            "representation");
        free(op);
        return MEMORY_ALLOCATION_ERROR;
    }
    op->func = table_equivalence;
    err = hash_table_set(operators, &representation, op);
    if (err) {
        free(op);
        string_free(representation);
        log_error("Failed to set equivalence operator to hash table");
        return err;
    }
    free(op);
    op = NULL;
    representation = NULL;

    // Sheffer Stroke (!)
    op = (operator_t *)malloc(sizeof(operator_t));
    if (op == NULL) {
        log_error("Memory allocation for Sheffer stroke operator error");
        return MEMORY_ALLOCATION_ERROR;
    }
    op->type = binary;
    representation = string_from("!");
    if (representation == NULL) {
        log_error(
            "Failed to allocate memory for Sheffer stroke operator "
            "representation");
        free(op);
        return MEMORY_ALLOCATION_ERROR;
    }
    op->func = table_sheffer_stroke;
    err = hash_table_set(operators, &representation, op);
    if (err) {
        free(op);
        string_free(representation);
        log_error("Failed to set Sheffer stroke operator to hash table");
        return err;
    }
    free(op);
    op = NULL;
    representation = NULL;

    // Webber Function (?)
    op = (operator_t *)malloc(sizeof(operator_t));
    if (op == NULL) {
        log_error("Memory allocation for Webber function operator error");
        return MEMORY_ALLOCATION_ERROR;
    }
    op->type = binary;
    representation = string_from("?");
    if (representation == NULL) {
        log_error(
            "Failed to allocate memory for Webber function operator "
            "representation");
        free(op);
        return MEMORY_ALLOCATION_ERROR;
    }
    op->func = table_webber_function;
    err = hash_table_set(operators, &representation, op);
    if (err) {
        free(op);
        string_free(representation);
        log_error("Failed to set Webber function operator to hash table");
        return err;
    }
    free(op);
    op = NULL;
    representation = NULL;

    return EXIT_SUCCESS;
}

err_t table_create_table_of_truth(const String postfix_exp,
                                  hash_table *operators) {
    if (postfix_exp == NULL || operators == NULL) {
        log_error("passed ptr is NULL");
        return DEREFERENCING_NULL_PTR;
    }

    u_list *operands_name = NULL;
    err_t err = 0;
    size_t i = 0, j = 0, operands_count = 0;
    u_list_node *current = NULL;
    String current_name = NULL, for_copy = NULL;
    int value = 0;
    hash_table *operands = NULL;
    int res = 0;

    err = u_list_init(&operands_name, sizeof(String *), table_u_list_free);
    if (err) {
        log_error("failed to create list");
        return err;
    }

    err = table_read_variables_to_list(postfix_exp, operators, operands_name);
    if (err) {
        u_list_free(operands_name);
        return err;
    }

    operands_count = operands_name->size;
    current = operands_name->first;
    while (current != NULL) {
        current_name = *(String *)current->data;
        printf("%c ", *current_name);
        current = current->next;
    }
    printf("F\n");

    for (i = 0; i < (1 << operands_count); ++i) {
        j = 0;
        hash_table_free(operands);
        err = hash_table_init(&operands, table_operands_keys_compare, djb2_hash,
                              sizeof(String *), sizeof(int),
                              table_operands_bucket_free);
        if (err) {
            log_error("failed to create hash table");
            u_list_free(operands_name);
            return err;
        }
        current = operands_name->first;
        while (current != NULL) {
            current_name = *(String *)current->data;
            value = (i & (1 << j)) != 0;

            for_copy = string_init();
            if (for_copy == NULL) {
                log_error("memory allocation error");
                u_list_free(operands_name);
                hash_table_free(operands);
                return MEMORY_ALLOCATION_ERROR;
            }

            err = string_cpy(&for_copy, &current_name);
            if (err) {
                log_error("memory allocation error");
                string_free(for_copy);
                u_list_free(operands_name);
                hash_table_free(operands);
                return err;
            }

            err = hash_table_set(operands, &for_copy, &value);
            if (err) {
                log_error("memory allocation error");
                string_free(for_copy);
                u_list_free(operands_name);
                hash_table_free(operands);
                return err;
            }
            for_copy = NULL;
            printf("%d ", value);
            j++;
            current = current->next;
        }
        err = calculate_postfix_expression(postfix_exp, &res, operators,
                                           operands);
        if (err) {
            u_list_free(operands_name);
            hash_table_free(operands);
            return err;
        }
        printf("%d\n", res == 0 ? 0 : 1);
    }

    printf("\n");

    u_list_free(operands_name);
    hash_table_free(operands);
    return EXIT_SUCCESS;
}

err_t table_read_variables_to_list(const String postfix_exp,
                                   hash_table *operators,
                                   u_list *operands_name) {
    if (postfix_exp == NULL || operators == NULL || operands_name == NULL) {
        log_error("passed ptr is NULL");
        return DEREFERENCING_NULL_PTR;
    }

    err_t err = 0;
    size_t i = 0;
    String token = NULL;
    char pe = 0, to_validate = 0;
    operator_t *op = NULL;
    String to_push = NULL;

    token = string_init();
    if (token == NULL) {
        log_error("failed to allocate memory for string");
        return MEMORY_ALLOCATION_ERROR;
    }

    for (i = 0; i < string_len(postfix_exp); ++i) {
        pe = postfix_exp[i];

        if (pe == ' ') {
            err = hash_table_get(operators, &token, (void **)&op);
            if (err != EXIT_SUCCESS && err != KEY_NOT_FOUND) {
                log_error("failed to get from hash table");
                string_free(token);
                return err;
            }
            if (err == KEY_NOT_FOUND) {  // is operand
                if (string_len(token) > 0 && !isdigit(token[0])) {
                    to_push = string_init();
                    if (to_push == NULL) {
                        log_error("failed to allocate memory");
                        string_free(token);
                        return MEMORY_ALLOCATION_ERROR;
                    }
                    err = string_cpy(&to_push, &token);
                    if (err) {
                        log_error("failed to cpy string");
                        string_free(token);
                        string_free(to_push);
                        return err;
                    }
                    err = u_list_push_back(operands_name, &to_push);
                    if (err) {
                        log_error("failed to push to list");
                        string_free(token);
                        string_free(to_push);
                        return err;
                    }
                    to_push = NULL;
                }
            }
            string_free(token);
            token = NULL;
            token = string_init();
            if (token == NULL) {
                log_error("memory allocation error");
                return MEMORY_ALLOCATION_ERROR;
            }
        } else {
            err = string_add(&token, pe);
            if (err) {
                log_error("string add failed");
                string_free(token);
                return err;
            }
        }
    }

    string_free(token);

    return EXIT_SUCCESS;
}
