#include "hash_table.h"
#include "hash_table_iterator.h"
#include "linked_list.h"
#include "list_iterator.h"
#include "utils.h"

typedef struct elem elem_l_t;

// stores where item is stored and quantity at that location
// for use in item->l
struct elem
{
    char *sl;
    int quant;
    int no_reserved: // number of items of this merch reserved in a cart
}

struct item
{
    char *name;
    char *description;
    int price;
    int total_stock;
    int total_reserved;
    ioopm_list_t *l;
};

// TODO: use hashmap instead of linked list?
// TODO: also use a hashmap in runtime of all the carts active

struct cart
{
    ioopm_list_t *l;
    int id;
    int total_cost; //add to this number everytime we add an item
}

typedef cart_elem cart_elem_t

struct cart_elem
{
    char *name;
    int quant;
}

// we need two HT:s 
// first one is for storing item names as keys and
// pointers to items as values

// second one is for storing where items are 
// stored using locations as keys and, lets say
// linked lists containing items as values

// OBS inserting new items with the same name
// will overwrite that data!

// FIX append to the list at that location instead
// of just naively inserting into the ht


item_t *db_create_item(char *name, char *desc, int price)
{
    item_t *item = calloc(1, sizeof(item_t));
    item->name = name;
    item->desc = desc;
    item->price = price;
    item->l = ioopm_list_create();
    return item;
}

void db_add_merch(ioopm_hash_table_t *ht_n_i, item_t *merch)
{
    ioopm_hash_table_insert(ht_n_i, merch->name, merch);
}