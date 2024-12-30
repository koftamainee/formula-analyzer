#ifndef TABLE_H_
#define TABLE_H_

#include <stdio.h>

#include "../libc/cstring.h"
#include "../libc/errors.h"

err_t process_table_file(FILE *fin);

err_t table_infix_to_postfix(const String infix_exp, String *postfix_exp);

#endif  // !TABLE_H_
