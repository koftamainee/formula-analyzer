#ifndef TABLE_H_
#define TABLE_H_

#include <stdio.h>

#include "../libc/cstring.h"
#include "../libc/errors.h"
#include "../libc/hash_table.h"
#include "cli.h"

err_t process_table_file(file_to_process *file);
err_t process_table_line(char *line, hash_table *operators);

err_t table_infix_to_postfix(const String infix_exp, String *postfix_exp);

err_t table_fill_hash_table_with_operators(hash_table *operators);

err_t table_create_table_of_truth(const String postfix_exp,
                                  hash_table *operators);
err_t table_read_variables_to_list(const String postfix_exp,
                                   hash_table *operators,
                                   u_list *operands_names);

#endif  // !TABLE_H_
