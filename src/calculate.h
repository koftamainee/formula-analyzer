#ifndef CALCULATE_H_
#define CALCULATE_H_

#include "../libc/cstring.h"
#include "../libc/errors.h"
#include "cli.h"

err_t process_calculate_file(FILE *fin);
err_t process_calculate_line(char *line);

err_t calculate_infix_to_postfix(const String infix_exp, String *postfix_exp);

#endif  // !CALCULATE_H_
