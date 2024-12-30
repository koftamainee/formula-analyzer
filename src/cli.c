#include "cli.h"

#include <stdlib.h>
#include <string.h>

#include "../libc/logger.h"

void file_to_process_free(void *f) {
    if (f == NULL) {
        return;
    }
    file_to_process *fp = f;
    fclose(fp->data);
    free(fp);
}

err_t parse_cli_arguments(u_list *files, int argc, char *argv[]) {
    if (files == NULL) {
        log_error("files ptr is NULL");
        return DEREFERENCING_NULL_PTR;
    }
    if (files == NULL) {
        log_error("argv ptr is NULL");
        return DEREFERENCING_NULL_PTR;
    }

    err_t err = 0;
    size_t i = 0;
    file_to_process fp;

    if (argc < 3) {  // at least one file and one flag
        log_error("Not enouth arguments");
        return NOT_ENOUTH_ARGUMENTS;
    }

    for (i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "--calculate") == 0 && i + 1 < argc) {
            fp.op = calculate;
            fp.data = fopen(argv[++i], "r");
            if (fp.data == NULL) {
                log_error("failed to open %s file", argv[i]);
                return OPENING_THE_FILE_ERROR;
            }
            err = u_list_insert(files, 0, &fp);
            if (err) {
                log_error("failed to push file to the list.");
                fclose(fp.data);
                return err;
            }
        } else if (strcmp(argv[i], "--table") == 0 && i + 1 < argc) {
            fp.op = table;
            fp.data = fopen(argv[++i], "r");
            if (fp.data == NULL) {
                log_error("failed to open %s file", argv[i]);
                return OPENING_THE_FILE_ERROR;
            }
            err = u_list_insert(files, 0, &fp);
            if (err) {
                log_error("failed to push file to the list.");
                fclose(fp.data);
                return err;
            }
        }
    }

    return EXIT_SUCCESS;
}
