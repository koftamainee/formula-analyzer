#ifndef CALCULATE_H_
#define CALCULATE_H_

#include "../libc/cstring.h"
#include "../libc/errors.h"
#include "../libc/hash_table.h"
#include "cli.h"

err_t process_calculate_file(file_to_process *file);
err_t process_calculate_line(char *line, hash_table *operators,
                             hash_table *operands);

err_t calculate_infix_to_postfix(const String infix_exp, String *postfix_exp);

err_t calculate_fill_hash_table_with_operators(hash_table *operators);

#endif  // !CALCULATE_H_
