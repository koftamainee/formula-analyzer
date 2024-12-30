#include "table.h"

#include <ctype.h>
#include <limits.h>
#include <stdlib.h>

#include "../libc/logger.h"
#include "postfix_notation.h"

err_t process_table_file(FILE *fin) {
    if (fin == NULL) {
        log_error("fin ptr is NULL");
        return DEREFERENCING_NULL_PTR;
    }

    return EXIT_SUCCESS;
}

int table_priorities(int c) {
    switch (c) {
        case '(':          // Скобка открытия
            return 10000;  // Максимальный приоритет
        case ')':          // Скобка закрытия
            return 9999;  // Немного меньше, чем открытие
        case '~':         // Отрицание
            return 9000;
        case '&':  // Конъюнкция
            return 8000;
        case '|':  // Дизъюнкция
            return 7000;
        case '>':  // Импликация
            return 6000;
        case '+':  // Коимпликация
            return 5000;
        case '#':  // Сложение по модулю 2 (вместо <>)
            return 4000;
        case '!':  // Штрих Шеффера
            return 3000;
        case '?':  // Функция Вебба
            return 2000;
        case '=':  // Эквивалентность
            return 1000;
        default:  // Переменная или константа
            return 0;
    }
}

int table_is_operator(int c) {
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '~' ||
        c == '^') {
        return 1;
    }
    return 0;
}

err_t table_infix_to_postfix(const String infix_exp, String *postfix_exp) {
    if (infix_exp == NULL || postfix_exp == NULL) {
        log_error("Passed ptr is NULL");
        return DEREFERENCING_NULL_PTR;
    }

    return infix_to_postfix(infix_exp, isalnum, table_is_operator,
                            table_priorities, postfix_exp);
}
