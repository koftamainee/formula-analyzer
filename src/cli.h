#ifndef CLI_H_
#define CLI_H_

#include <stdio.h>

#include "../libc/errors.h"
#include "../libc/u_list.h"

typedef enum { calculate, table } file_operation;

typedef struct {
    FILE *data;
    file_operation op;
    char *filename;
} file_to_process;

err_t parse_cli_arguments(u_list *files, int argc, char *argv[]);

void file_to_process_free(void *f);

#endif  // !CLI_H_
