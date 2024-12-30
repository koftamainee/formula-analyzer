#ifndef POSTFIX_NOTATION_H_
#define POSTFIX_NOTATION_H_

#include "../libc/cstring.h"
#include "../libc/errors.h"

typedef enum { unary, binary } operation_type;

typedef struct operation {
    char representation;
    operation_type type;
    int (*func)(int, ...);
} operation;

err_t infix_to_postfix(const String infix_exp, int (*is_operand)(int c),
                       int (*is_operator)(int c), int (*priority_mapper)(int c),
                       String *postfix_exp);
err_t calculate_postfix_expression(const String postfix_exp,
                                   int *expression_result,
                                   size_t operators_count, ...);
#endif  // !POSTFIX_NOTATION_H_
