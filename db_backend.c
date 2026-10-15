#include "hash_table.h"
#include "hash_table_iterator.h"
#include "linked_list.h"
#include "list_iterator.h"
#include "utils.h"
#include <stdlib.h>
#include "db_backend.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>


item_t *db_create_item(char *name, char *desc, int price)
{
    item_t *item = calloc(1, sizeof(item_t));
    item->name = name;
    item->desc = desc;
    item->price = price;
    item->l = ioopm_list_create();
    return item;
}

stock_entry_t *db_create_stock_entry(char *sl, int quant)
{
    stock_entry_t *entry = calloc(1, sizeof(stock_entry_t));
    entry->sl = sl;
    entry->quant = quant;
    return entry;
}

db_status_t db_add_merch(ioopm_hash_table_t *ht_n_i, item_t *merch)
{
    if (ioopm_hash_table_has_key(ht_n_i, string_elem(merch->name)))
    {
        return DB_MERCH_EXISTS;
    }
    ioopm_hash_table_insert(ht_n_i, string_elem(merch->name), ptr_elem(merch));
    return DB_OK;
}

db_status_t db_get_item(ioopm_hash_table_t *ht_n_i, char *name, item_t **result)
{
    elem_t holder;

    if (ioopm_hash_table_lookup(ht_n_i, string_elem(name), &holder))
    {
        *result = holder.p;
        return DB_OK;
    }
    else
    {
        return DB_NO_SUCH_MERCH;
    }
}

static void item_replenish(item_t *item, char *sl, int quant, bool exists)
{

    ioopm_list_t *ll = item->l;
    if (exists) // sl is in list meaning we have to find it
    {
        ioopm_list_iterator_t *iter = ioopm_list_iterator_create(ll);
        stock_entry_t *curr = ioopm_list_iterator_current(iter).p;
        while (!ioopm_list_iterator_at_end(iter))
        {
            curr = ioopm_list_iterator_current(iter).p;
            if (strcmp(curr->sl, sl) == 0)
            {
                // this sl needs to be updated
                curr->quant += quant;
                break;
            }
            // else look for next node
            ioopm_list_iterator_advance(iter);
        }
        ioopm_list_iterator_destroy(iter);
    }
    else //sl does not exist in list we just append
    {
        stock_entry_t *stock_entry = db_create_stock_entry(sl, quant);
        ioopm_list_append(ll, ptr_elem(stock_entry));
    }
}

db_status_t db_replenish(ioopm_hash_table_t *ht_n_i,
                         ioopm_hash_table_t *ht_sl_n,
                         char *name, int quant, char *sl)
{
    item_t *holder = NULL;

    if (db_get_item(ht_n_i, name, &holder) != DB_OK)
    {
        return DB_NO_SUCH_MERCH;
    }

    elem_t res;

    if (ioopm_hash_table_lookup(ht_sl_n, string_elem(sl), &res))
    {
        char *item_name = res.s;

        if (strcmp(item_name, name) == 0)
        {
            item_replenish(holder, sl, quant, true);
            return DB_OK;
        }
        else
        {
            return DB_LOCATION_TAKEN;
        }
    }
    else
    {
        ioopm_hash_table_insert(ht_sl_n, string_elem(sl), string_elem(name));
        item_replenish(holder, sl, quant, false);
        return DB_OK;
    }
}

void db_ht_destroy(ioopm_hash_table_t *ht)
{
    ioopm_hash_table_iterator_t *it = ioopm_hash_table_iterator_create(ht);

    while (!ioopm_hash_table_iterator_at_end(it))
    {
        elem_t value = ioopm_hash_table_iterator_current_value(it);
        item_t *item = value.p;

        ioopm_list_destroy(item->l);
        free(item);

        ioopm_hash_table_iterator_advance(it);
    }

    ioopm_hash_table_iterator_destroy(it);
    ioopm_hash_table_destroy(ht);
}