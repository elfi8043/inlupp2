#include "hash_table.h"
#include "hash_table_iterator.h"
#include "linked_list.h"
#include "list_iterator.h"
#include "utils.h"
#include "stdlib.h"
#include "db_backend.h"
#include <stdbool.h>

item_t *db_create_item(char *name, char *desc, int price)
{
    item_t *item = calloc(1, sizeof(item_t));
    item->name = name;
    item->desc = desc;
    item->price = price;
    item->l = ioopm_list_create();
    return item;
}

stock_entry_t *db_create_stock_entry(sl, quant)
{
    stock_entry_t *entry = calloc(1, sizeof(stock_entry_t));
    entry->sl = sl;
    entry->quant = quant;
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

db_status_t db_get_item(ioopm_hash_table_t *ht_n_i, char *name, item_t *holder)
{
    if(ioopm_hash_table_lookup(ht_n_i, string_elem(name), &ptr_elem(holder)))
    {
        return DB_OK;
    } else
    {
        return DB_NO_SUCH_MERCH;
    }

}

static void item_replenish(item_t *item, char *sl, int quant, bool exists)
{
    ioopm_list_t *ll = holder->l;
    if (exists) // sl is in list meaning we have to find it
    {
        ioopm_list_iterator_t *iter = ioopm_list_iterator_create(ll);
        while (!ioopm_hash_table_iterator_at_end(iter))
        {
            curr = ioopm_list_iterator_current_value(iter);
            
        }
    }

}

db_status_t db_replenish(ioopm_hash_table_t *ht_n_i, ioopm_hash_table_t *ht_sl_n, char *name, int quant, char *sl)
{
    //update the items list
    
    // 1, get the item from db
    item_t *holder = db_create_holder();
    if(db_get_item(ht_n_i, name, holder) == DB_OK)
    {
        char *res;
        //2. check if sl in use
        if (ioopm_hash_table_lookup(ht_sl_n, string_elem(sl), string_elem(res)))
        {
            // If sl in use we check if it is in use by the same item
            if (strcmp(res, name) == 0)
            {
                // if in use by same item we replenish
                item_replenish(holder, sl, quant, true);
            }
        }

        
        // add it to ht_sl_n
    } 

}

item_t *db_create_holder(void)
{
    return calloc(1, sizeof(item_t));
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