#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "../libc/logger.h"
#include "calculate.h"
#include "cli.h"
#include "table.h"

int is_operator(int c) {
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '~' ||
        c == '^')
        return 1;
    return 0;
}

int priorities(int operator) {
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

int main(int argc, char *argv[]) {
    err_t err = 0;
    u_list *files = NULL;
    u_list_node *current = NULL;
    file_to_process *current_data = NULL;

    err = logger_start();
    if (err) {
        return err;
    }
    log_set_level(LOG_WARN);
    err = u_list_init(&files, sizeof(file_to_process), file_to_process_free);
    if (err) {
        u_list_free(files);
        return err;
    }
    err = parse_cli_arguments(files, argc, argv);
    if (err) {
        u_list_free(files);
        return err;
    }

    current = files->first;
    while (current != NULL) {
        current_data = current->data;
        switch (current_data->op) {
            case calculate:
                err = process_calculate_file(current_data->data);
                if (err) {
                    u_list_free(files);
                    return err;
                }
                break;
            case table:
                err = process_table_file(current_data->data);
                if (err) {
                    u_list_free(files);
                    return err;
                }
                break;
        }
        current = current->next;
    }

    u_list_free(files);
    // i ain't closing logger bc i use it only for errors,
    // check logger.c/logger_start()
    return EXIT_SUCCESS;
}
