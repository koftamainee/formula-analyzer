#include "calculate.h"

#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "../libc/cstring.h"
#include "../libc/logger.h"
#include "cli.h"
#include "expression_tree.h"
#include "postfix_notation.h"

int calculate_is_op_binary(const String op) {
    const char *valid_binary_operators[] = {"+", "*", "/",   "%",
                                            "~", "^", "cos", "||"};
    size_t num_operators =
        sizeof(valid_binary_operators) / sizeof(valid_binary_operators[0]);
    size_t i = 0;

    if (strncmp(op, "~", 1) == 0) {
        return 0;  // is unary
    }

    for (i = 0; i < num_operators; i++) {
        if (strncmp(op, valid_binary_operators[i],
                    strlen(valid_binary_operators[i])) == 0) {
            return 1;
        }
    }
    return -1;  // is not an operator
}

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
    const char *valid_operators[] = {"+", "-", "*",   "/", "%",
                                     "~", "^", "cos", "||"};
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
        err = process_calculate_line(line);
        if (err != EXIT_SUCCESS && err != INVALID_BRACES) {
            if (fout != NULL) {
                fclose(fout);
            }
            return err;
        }
        if (err == INVALID_BRACES) {
            if (fout == NULL) {
                sprintf(error_filename, "%s.errors", file->filename);
                fout = fopen(error_filename, "w");
                if (fout == NULL) {
                    log_error("Error while openning file for errors");
                    return OPENING_THE_FILE_ERROR;
                }
            }
            fprintf(fout, "%s : %zu - Invalid braces placement error.\n",
                    file->filename, current_line);
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

    return EXIT_SUCCESS;
}

err_t process_calculate_line(char *line) {
    if (line == NULL) {
        log_error("passed ptr is NULL");
        return DEREFERENCING_NULL_PTR;
    }

    err_t err = 0;
    String infix = NULL, postfix = NULL;
    expression_tree *tree = NULL;

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
