#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include "utils.h"

extern char *strdup(const char *);

void clear_input_buffer(void)
{
    int c;
    do
    {
        c = getchar();
    } while (c != '\n' && c != EOF);
}

int read_string(char *buf, int buf_size)
{
    // initiera räknare
    int read_char = 0;
    // medan read_char är [0, buf_size -1], läs till buf:
    do
    {
        char c = getchar();

        // om c är \n = klar + markera slut + töm ej buffer
        if (c == '\n')
        {
            buf[read_char] = '\0';
            return read_char;
        }

        buf[read_char] = c; // lägg till karaktär
        read_char += 1;

    } while (read_char < buf_size - 1);
    // buffer slut! markera slut + töm buffer
    buf[read_char] = '\0';
    clear_input_buffer();
    return read_char;
}

answer_t ask_question(char *question, check_func *check, convert_func *convert)
{
    char buf[255];
    do 
    {
        printf("%s", question);
        read_string(buf, 255);
        bool success = check(buf);
        if (success) 
        {
            break;
        }

    } while(true);

    return convert(buf);
}

bool is_number(char *str)
{
    int len = strlen(str);
    if (len == 0)
    {
        return false;
    }
    for (int i = 0; i < len; i++)
    {
        if (isdigit(str[i]) == false)
        {
            if (i == 0 && str[i] == '-')
            {
                continue;
            }
            return false;
        }
    }
    return true;
}

bool not_empty(char *str)
{
  return strlen(str) > 0;
}

bool is_float(char *str)
{
    int i = 0;
    bool decimal = false;
    bool digit = false;

    if (str[i] == '-')
        i++;

    for (; str[i] != '\0'; i++)
    {
        if (str[i] == '.')
        {
            if (decimal)
                return false;

            decimal = true;
        }
        else if (isdigit(str[i]))
        {
            digit = true;
        }
        else
        {
            return false;
        }
    }

    return digit;
}

bool is_shelf(char *shelf)
{
    int len = strlen(shelf);
    if (len > 3 || !isalpha(shelf[0]))
    {
        return false;
    }
    if (len == 0)
    {
        return false;
    }
    for (int i = 0; i < len; i++)
    {
        if (isdigit(shelf[i]) == false)
        {
            if (i == 0 && isalpha(shelf[i]))
            {
                continue;
            }
            return false;
        }
    }
    return true;
}


answer_t make_float(char *str)
{
  answer_t a;                // skapa ett oinitierat answer_t-värde
  a.float_value = atof(str); // gör det till en float, via atof
  return a;                  // returnera värdet
}

double ask_question_float(char *question)
{
  return ask_question(question, is_float, make_float).float_value;
}

int ask_question_int(char *question)
{
  answer_t answer = ask_question(question, is_number, (convert_func *) atoi);
  return answer.int_value; // svaret som ett heltal
}

char *ask_question_string(char *question)
{
  return ask_question(question, not_empty, (convert_func *) strdup).string_value;
}

char *ask_question_shelf(char *question)
{
    return ask_question(question, is_shelf, (convert_func *) strdup).string_value;
}