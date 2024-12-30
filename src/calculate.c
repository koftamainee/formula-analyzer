#include "calculate.h"

#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "../libc/cstring.h"
#include "../libc/logger.h"
#include "postfix_notation.h"

err_t process_calculate_file(FILE *fin) {
    if (fin == NULL) {
        log_error("fin ptr is NULL");
        return DEREFERENCING_NULL_PTR;
    }

    char line[BUFSIZ];
    err_t err = 0;
    size_t len = 0;

    while (fgets(line, sizeof(line), fin)) {
        len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';  // delete '\n' symbol
        }
        err = process_calculate_line(line);
        if (err) {
            // TODO handle me correctly pls
            return err;
        }
    }

    return EXIT_SUCCESS;
}

err_t process_calculate_line(char *line) {
    if (line == NULL) {
        log_error("line ptr is NULL");
        return DEREFERENCING_NULL_PTR;
    }

    err_t err = 0;
    String infix = NULL, postfix = NULL;

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
        // TODO create file and print runtime errors in there
    }

    printf("Source: (inf) ");
    string_print(infix);
    printf("\nConverted: (post) ");
    string_print(postfix);
    printf("\n\n");

    // TODO: build arithmetic tree, calculate exp

    string_free(infix);
    string_free(postfix);

    return EXIT_SUCCESS;
}

int calculate_priorities(int operator) {
    switch (operator) {
        case '+':
        case '-':
            return 0;
        case '*':
        case '/':
        case '%':
            return 1;
        case '~':
            return 2;
        case '^':
            return 3;
        case '(':
            return INT_MIN;
        default:
            return -1;
    }
}

int calculate_is_operator(int c) {
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '~' ||
        c == '^') {
        return 1;
    }
    return 0;
}

err_t calculate_infix_to_postfix(const String infix_exp, String *postfix_exp) {
    return infix_to_postfix(infix_exp, isalnum, calculate_is_operator,
                            calculate_priorities, postfix_exp);
}
