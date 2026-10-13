#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include "utils.h"
#include <time.h>
#include <string.h>
#include <ctype.h>

struct item
{
    char *name;
    char *desc;
    int price;
    char *shelf;
};

typedef struct item item_t;

void print_item(item_t *item)
{
    printf("Name: %s\n", item->name);
    printf("Desc: %s\n", item->desc);
    printf("Price: %d.%02d\n",item->price / 100, item->price % 100); //att använda %02d var chatgpts idé
    printf("Shelf: %s\n", item->shelf);
}

item_t make_item(char *name, char *desc, int price, char *shelf)
{
    return (item_t){.name = name, .desc = desc, .price = price, .shelf = shelf};
}

item_t input_item()
{
    char *name = ask_question_string("Name: ");
    char *desc = ask_question_string("Desc: ");
    int price = ask_question_int("Price: ");
    char *shelf = ask_question_shelf("Shelf: ");
    return make_item(name, desc, price, shelf);
}

char * magick(char *array1[], char *array2[], char *array3[], int len)
{
    char buf[255];
    buf[0] = '\0'; // För att strcat ska fungera behöver buf vara en null terminerad sträng
    int n1 = random() % len;
    int n2 = random() % len;
    int n3 = random() % len;
    char *word_1 = array1[n1];
    strcat(buf, word_1); 
    strcat(buf, "-");
    char *word_2 = array2[n2];
    strcat(buf, word_2);
    strcat(buf, " ");
    char *word_3 = array3[n3];
    strcat(buf, word_3);
    return strdup(buf);
}

void list_db(item_t *items, int no_items)
{
    if (no_items == 0)
    {
        puts("Empty DB");
    }
    for (int i = 0; i < no_items; i++)
    {
        printf("%d. %s\n", i + 1, items[i].name);
    }
}

void edit_db(item_t *items, int db_siz)
{   
    if (db_siz > 0)
    {
        list_db(items, db_siz);
        int i;
        while (true)
        {
            i = ask_question_int("Vilken vara vill du editera? ");
            i--;
            if (i < 0 || i > db_siz)
            {
                printf("Varan existerar inte, ange 0 - %d\n", db_siz);
            } else 
            {
                break;
            }
        }
        printf("#%d\n", i);
        print_item(&items[i]);
        printf("Ange ny data: \n");
        items[i] = input_item();
        puts("Success, new db: ");
        list_db(items, db_siz);
    } else {puts("empty db");}
}   

bool is_choice(char *c)
{   
    if (strlen(c) != 1) 
    {  
        return false;
    }
    int d = c[0];
    d = toupper(d);
    if (d == 'L' | d == 'T' | d == 'R' | d == 'G'| d == 'H' | d == 'A')
    {
        return true;
    } else 
    {
        return false;
    }
}

answer_t convert_choice(char *c)
{
    answer_t a;
    a.character_value = c[0];
    return a;
}

char ask_question_menu()
{
    char *menu = "\n[L]ägga till en vara\n"
                 "[T]a bort en vara\n"
                 "[R]edigera en vara\n"
                 "Ån[g]ra senaste ändringen\n"
                 "Lista [h]ela varukatalogen\n"
                 "[A]vsluta\n\n"
                 "> ";

   return ask_question(menu, is_choice, convert_choice).character_value;
}

void add_item_to_db(item_t *items, int *db_siz)
{
    item_t item = input_item();
    items[*db_siz] = item;
    *db_siz += 1;
}
 
void remove_item_from_db(item_t *items, int *db_siz)
{
    if (*db_siz > 0) 
    {
        list_db(items, *db_siz);
        int i;
        while (true)
        {
            i = ask_question_int("Vilken vara vill du ta bort? ");
            i--;
            if (i < 0 || i > *db_siz)
            {
                printf("Varan existerar inte, ange 0 - %d\n", *db_siz);
            } else 
            {
                break;
            }
        }
        // vi vill alltså ta bort det i:te elementet genom att
        // kopiera över i+1 til i, i+2 till i + 1 osv 
        if (i == *db_siz)
        {
          *db_siz -= 1;
        } else {
            for (int j = i; j < *db_siz; j++)
            {
                items[j] = items[j +1];
            }
        *db_siz -= 1;
    }
    } else {puts("empty db");}
}   

void event_loop(item_t *db, int *db_siz)
{
    bool cont = true;
    while (cont)
    {
    char c = toupper(ask_question_menu());
    switch (c)
    {
        case 'L':
        add_item_to_db(db, db_siz);
        break;
        
        case 'T':
        remove_item_from_db(db, db_siz);
        break;

        case 'R':
        edit_db(db, *db_siz);
        break;

        case 'G':
        puts("Not yet implemented");
        break;

        case 'H':
        list_db(db, *db_siz);
        break;

        case 'A':
        cont = false;
        break;
    }
    }
}

int main(int argc, char *argv[])
{
    item_t db[16]; // Array med plats för 16 varor
    int db_siz = 0; // Antalet varor i arrayen just nu
    event_loop(db, &db_siz);
    return 0;
}
