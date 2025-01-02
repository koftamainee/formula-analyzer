#ifndef POSTFIX_NOTATION_H_
#define POSTFIX_NOTATION_H_

#include "../libc/cstring.h"
#include "../libc/errors.h"
#include "../libc/hash_table.h"

typedef enum { unary, binary } operator_type;

typedef struct {
    operator_type type;
    int (*func)(int, ...);
} operator_t;

err_t infix_to_postfix(const String infix_exp, int (*is_operand)(int c),
                       int (*is_operator)(const char *op),
                       int (*priority_mapper)(const String op),
                       String *postfix_exp);

err_t calculate_postfix_expression(const String postfix_exp,
                                   int *expression_result,
                                   hash_table *operators, hash_table *operands);
#endif  // !POSTFIX_NOTATION_H_
