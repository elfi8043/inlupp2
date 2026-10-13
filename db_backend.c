#include "hash_table.h"
#include "hash_table_iterator.h"
#include "linked_list.h"
#include "list_iterator.h"
#include "utils.h"
#include "stdlib.h"
#include "db_backend.h"

item_t *db_create_item(char *name, char *desc, int price)
{
    item_t *item = calloc(1, sizeof(item_t));
    item->name = name;
    item->desc = desc;
    item->price = price;
    item->l = ioopm_list_create();
    return item;
}

db_status_t db_add_merch(ioopm_hash_table_t *ht_n_i, item_t *merch)
{
    if (ioopm_hash_table_has_key(ht_n_i, merch->name))
    {
        return DB_MERCH_EXISTS;
    }
    ioopm_hash_table_insert(ht_n_i, merch->name, ptr_elem(merch));
    return DB_OK;
}