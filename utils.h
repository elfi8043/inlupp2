#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

typedef union {
    int int_value;
    float float_value;
    char *string_value;
    char character_value;
} answer_t;

typedef bool check_func(char *);
typedef answer_t convert_func(char *);

void clear_input_buffer(void);

int read_string(char *buf, int buf_size);

answer_t ask_question(char *question, check_func *check, convert_func *convert);

bool is_number(char *str);

bool not_empty(char *str);

bool is_float(char *str);

answer_t make_float(char *str);

double ask_question_float(char *question);

int ask_question_int(char *question);

char *ask_question_string(char *question);

char *ask_question_shelf(char *question);

bool is_shelf(char *shelf);

#endif