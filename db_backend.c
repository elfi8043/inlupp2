#include "db_backend.h"


typedef struct
{
    char *name;
    int quant;
} cart_elem_t;

struct cart
{
    ioopm_list_t *l;
    int id;
    int total_cost;
};

/////////////////////
// HJÄLPFUNKTIONER
/////////////////////

/**
 * Skapar en vara och initierar dess fält.
 * Namn och beskrivning kopieras så att varan äger sina strängar.
 */
item_t *db_create_item(char *name, char *desc, int price)
{
    item_t *item = calloc(1, sizeof(item_t));
    item->name = strdup(name);
    item->desc = strdup(desc);
    item->price = price;
    item->l = ioopm_list_create();

    return item;
}


/**
 * Frigör en lagerlista, inklusive lagerplatsernas strängar
 * och tillhörande lagerposter.
 */
static void destroy_stock_list(ioopm_list_t *list)
{
    ioopm_list_iterator_t *it = ioopm_list_iterator_create(list);

    while (!ioopm_list_iterator_at_end(it))
    {
        stock_entry_t *curr_sl = ioopm_list_iterator_current(it).p;

        free(curr_sl->sl);
        free(curr_sl);

        ioopm_list_iterator_advance(it);
    }

    ioopm_list_iterator_destroy(it);
    ioopm_list_destroy(list);
}


/**
 * Frigör en vara och all data som varan äger.
 */
static void destroy_item(item_t *item)
{
    free(item->name);
    free(item->desc);
    destroy_stock_list(item->l);
    free(item);
}


/**
 * Skapar en lagerpost med en kopia av lagerplatsens namn
 * och anger hur många enheter som finns på platsen.
 */
static stock_entry_t *db_create_stock_entry(char *sl, int quant)
{
    stock_entry_t *entry = calloc(1, sizeof(stock_entry_t));
    entry->sl = strdup(sl);
    entry->quant = quant;

    return entry;
}

/////////////////////
// VARUHANTERING
/////////////////////

/**
 * Lägger till en vara i hashtabellen.
 * Om varan redan finns frigörs den nya varan och ett fel returneras.
 */
db_status_t db_add_merch(ioopm_hash_table_t *ht_n_i, item_t *merch)
{
    if (ioopm_hash_table_has_key(ht_n_i, string_elem(merch->name)))
    {
        destroy_item(merch);
        return DB_MERCH_EXISTS;
    }

    ioopm_hash_table_insert(
        ht_n_i,
        string_elem(merch->name),
        ptr_elem(merch)
    );

    return DB_OK;
}


/**
 * Ändrar en varas namn, beskrivning och pris.
 * Befintligt lager kopieras och återskapas efter att den gamla
 * varan tagits bort.
 */
db_status_t db_edit_merch(ioopm_hash_table_t *ht_n_i,
                          ioopm_hash_table_t *ht_sl_n,
                          char *old_name,
                          char *name,
                          char *desc,
                          int price)
{
    elem_t to_remove;

    if (!ioopm_hash_table_lookup(
            ht_n_i, string_elem(old_name), &to_remove))
    {
        return DB_NO_SUCH_MERCH;
    }

    item_t *item = to_remove.p;
    bool renamed = strcmp(old_name, name) != 0;

    if (renamed && ioopm_hash_table_has_key(ht_n_i, string_elem(name)))
    {
        return DB_MERCH_EXISTS;
    }

    // Kundvagnar sparar namnet, så det får inte ändras medan varan är reserverad
    if (renamed && item->total_reserved > 0)
    {
        return DB_MERCH_IN_CART;
    }

    item_t *item_to_insert = db_create_item(name, desc, price);
    item_to_insert->total_reserved = item->total_reserved;

    // Kopiera de befintliga lagerposterna till en tillfällig lista.
    // Det gör att informationen finns kvar när den gamla varan tas bort.
    ioopm_list_t *temp_list = ioopm_list_create();
    ioopm_list_iterator_t *it =
        ioopm_list_iterator_create(item->l);

    while (!ioopm_list_iterator_at_end(it))
    {
        stock_entry_t *curr_sl = ioopm_list_iterator_current(it).p;
        stock_entry_t *copied_sl =
            db_create_stock_entry(curr_sl->sl, curr_sl->quant);

        ioopm_list_append(temp_list, ptr_elem(copied_sl));
        ioopm_list_iterator_advance(it);
    }

    ioopm_list_iterator_destroy(it);

    // Ta bort den gamla varan och lägg till den uppdaterade varan.
    db_remove_merch(ht_n_i, ht_sl_n, old_name);
    db_add_merch(ht_n_i, item_to_insert);

    // Återskapa lagret för den uppdaterade varan.
    ioopm_list_iterator_t *it2 =
        ioopm_list_iterator_create(temp_list);

    while (!ioopm_list_iterator_at_end(it2))
    {
        stock_entry_t *curr_sl = ioopm_list_iterator_current(it2).p;

        assert(db_replenish(
                   ht_n_i, ht_sl_n, name,
                   curr_sl->quant, curr_sl->sl) == DB_OK);

        ioopm_list_iterator_advance(it2);
    }

    ioopm_list_iterator_destroy(it2);

    // Frigör den tillfälliga listan och dess kopierade lagerposter.
    destroy_stock_list(temp_list);

    return DB_OK;
}


/**
 * Hämtar en vara utifrån dess namn.
 * Resultatpekaren uppdateras om varan hittas.
 */
db_status_t db_get_item(ioopm_hash_table_t *ht_n_i,
                        char *name,
                        item_t **result)
{
    elem_t holder;

    if (ioopm_hash_table_lookup(
            ht_n_i, string_elem(name), &holder))
    {
        *result = holder.p;
        return DB_OK;
    }
    else
    {
        return DB_NO_SUCH_MERCH;
    }
}


/**
 * Ökar lagersaldot för en vara och uppdaterar antalet
 * om lagerplatsen redan finns i varans lagerlista.
 */
static void item_replenish(item_t *item, char *sl, int quant)
{
    item->total_stock += quant;

    ioopm_list_t *ll = item->l;
    ioopm_list_iterator_t *iter =
        ioopm_list_iterator_create(ll);

    while (!ioopm_list_iterator_at_end(iter))
    {
        stock_entry_t *curr = ioopm_list_iterator_current(iter).p;

        if (strcmp(curr->sl, sl) == 0)
        {
            // Lagerplatsen finns redan: uppdatera antalet.
            curr->quant += quant;
            break;
        }

        // Fortsätt söka bland nästa lagerposter.
        ioopm_list_iterator_advance(iter);
    }

    ioopm_list_iterator_destroy(iter);
}


/**
 * Fyller på lagret för en vara på en angiven lagerplats.
 * Returnerar fel om varan saknas eller lagerplatsen används
 * av en annan vara.
 */
db_status_t db_replenish(ioopm_hash_table_t *ht_n_i,
                         ioopm_hash_table_t *ht_sl_n,
                         char *name,
                         int quant,
                         char *sl)
{
    item_t *holder = NULL;

    if (db_get_item(ht_n_i, name, &holder) != DB_OK)
    {
        return DB_NO_SUCH_MERCH;
    }

    elem_t res;

    if (ioopm_hash_table_lookup(
            ht_sl_n, string_elem(sl), &res))
    {
        char *item_name = res.s;

        // Kontrollera att lagerplatsen tillhör den aktuella varan.
        if (strcmp(item_name, holder->name) == 0)
        {
            item_replenish(holder, sl, quant);
            return DB_OK;
        }
        else
        {
            return DB_LOCATION_TAKEN;
        }
    }
    else
    {
        // Skapa en ny lagerpost som varan äger.
        stock_entry_t *entry = db_create_stock_entry(sl, quant);
        holder->total_stock += quant; 
        ioopm_list_append(holder->l, ptr_elem(entry));

        // Registrera lagerplatsen och varans namn i hashtabellen.
        // Hashtabellen får egna kopior av båda strängarna.
        ioopm_hash_table_insert(
            ht_sl_n,
            string_elem(strdup(entry->sl)),
            string_elem(strdup(holder->name))
        );

        return DB_OK;
    }
}


/**
 * Tar bort en vara och frigör dess tillhörande lagerplatser.
 */
db_status_t db_remove_merch(ioopm_hash_table_t *ht_n_i,
                            ioopm_hash_table_t *ht_sl_n,
                            char *name)
{

    item_t *check_item = NULL;
    if (db_get_item(ht_n_i, name, &check_item) != DB_OK)
    {
        return DB_NO_SUCH_MERCH;
    }

    // 2. Vägra om den ligger i en kundvagn
    if (check_item->total_reserved > 0)
    {
        return DB_MERCH_IN_CART;
    }

    elem_t removed_val;
    elem_t removed_key;

    // Ta bort frpn ht_
    if (!ioopm_hash_table_remove(
            ht_n_i, string_elem(name),
            &removed_key, &removed_val))
    {
        return DB_NO_SUCH_MERCH;
    }

    item_t *item = removed_val.p;

    // Ta bort varans lagerplatser från lagerplatstabellen.
    ioopm_list_iterator_t *it =
        ioopm_list_iterator_create(item->l);

    while (!ioopm_list_iterator_at_end(it))
    {
        stock_entry_t *curr = ioopm_list_iterator_current(it).p;
        elem_t removed_key;
        elem_t removed_value;

        if (ioopm_hash_table_remove(
                ht_sl_n, string_elem(curr->sl),
                &removed_key, &removed_value))
        {
            free(removed_key.s);
            free(removed_value.s);
        }

        ioopm_list_iterator_advance(it);
    }

    ioopm_list_iterator_destroy(it);

    // Frigör varan och dess tillhörande data.
    destroy_item(item);

    return DB_OK;
}


/**
 * Frigör båda hashtabellerna och deras dynamiskt allokerade data.
 */
void db_ht_destroy(ioopm_hash_table_t *ht_n_i,
                   ioopm_hash_table_t *ht_sl_n)
{
    // Frigör alla varor i varuhashtabellen.
    ioopm_hash_table_iterator_t *it =
        ioopm_hash_table_iterator_create(ht_n_i);

    while (!ioopm_hash_table_iterator_at_end(it))
    {
        elem_t value = ioopm_hash_table_iterator_current_value(it);
        item_t *item = value.p;

        destroy_item(item);
        ioopm_hash_table_iterator_advance(it);
    }

    ioopm_hash_table_iterator_destroy(it);
    ioopm_hash_table_destroy(ht_n_i);

    // Frigör alla nycklar och värden i lagerplatstabellen.
    ioopm_hash_table_iterator_t *it2 =
        ioopm_hash_table_iterator_create(ht_sl_n);

    while (!ioopm_hash_table_iterator_at_end(it2))
    {
        elem_t value = ioopm_hash_table_iterator_current_value(it2);
        elem_t key = ioopm_hash_table_iterator_current_key(it2);

        free(value.s);
        free(key.s);

        ioopm_hash_table_iterator_advance(it2);
    }

    ioopm_hash_table_iterator_destroy(it2);
    ioopm_hash_table_destroy(ht_sl_n);
}


/////////////////////
// KUNDVAGNSFUNKTIONER
/////////////////////

/**
 * Skapar en kundvagn och tilldelar den ett ID.
 */
cart_t *db_create_cart(int *no_carts)
{
    cart_t *cart = calloc(1, sizeof(cart_t));
    cart->l = ioopm_list_create();

    *no_carts += 1;
    cart->id = *no_carts;

    return cart;
}

static cart_elem_t *find_cart_elem(cart_t *cart, char *name) {
    ioopm_list_iterator_t *iter =
        ioopm_list_iterator_create(cart->l);

    while (!ioopm_list_iterator_at_end(iter))
    {
        cart_elem_t *curr = ioopm_list_iterator_current(iter).p;

        if (strcmp(curr->name, name) == 0)
        {
            ioopm_list_iterator_destroy(iter);
            return curr;
        }
        ioopm_list_iterator_advance(iter);
    }
    ioopm_list_iterator_destroy(iter);
    return NULL;
    
}


int db_cart_id(cart_t *cart)
{
    return cart->id;
}

int db_cart_cost(cart_t *cart)
{
    return cart->total_cost;
}

size_t db_cart_size(cart_t *cart)
{
    return ioopm_list_size(cart->l);
}

int db_cart_quantity(cart_t *cart, char *name)
{
    cart_elem_t *elem = find_cart_elem(cart, name);
    if (elem) {
        return elem->quant;
    }
    return 0;
}

void db_cart_apply(cart_t *cart, cart_row_fn *fn)
{
    ioopm_list_iterator_t *it = ioopm_list_iterator_create(cart->l);
    while (!ioopm_list_iterator_at_end(it))
    {
        cart_elem_t *elem = ioopm_list_iterator_current(it).p;
        fn(elem->name, elem->quant);
        ioopm_list_iterator_advance(it);
    }
    ioopm_list_iterator_destroy(it);
}


/**
 * Tar bort en kundvagn och frigör dess lista.
 */
void db_destroy_cart(cart_t *cart)
{
    // remove allocted strings from cart elem names
    ioopm_list_iterator_t *iter = ioopm_list_iterator_create(cart->l);
    while (!ioopm_list_iterator_at_end(iter))
    {
        cart_elem_t *elem = ioopm_list_iterator_current(iter).p;
        free(elem->name);
        free(elem);
        ioopm_list_iterator_advance(iter);
    }
    ioopm_list_iterator_destroy(iter);
    ioopm_list_destroy(cart->l);
    free(cart);
}

void db_remove_cart(ioopm_hash_table_t *ht_n_i, cart_t *cart)
{
    ioopm_list_iterator_t *iter = ioopm_list_iterator_create(cart->l);
    while (!ioopm_list_iterator_at_end(iter))
    {
        cart_elem_t *elem = ioopm_list_iterator_current(iter).p;
        item_t *item = NULL;
        if (db_get_item(ht_n_i, elem->name, &item) == DB_OK)
        {
            item->total_reserved -= elem->quant;
        }
        ioopm_list_iterator_advance(iter);
    }
    ioopm_list_iterator_destroy(iter);

    db_destroy_cart(cart);
}

db_status_t db_add_to_cart(cart_t *cart, item_t *item, int quant) {
    if (item->total_reserved + quant > item->total_stock) {
        return DB_INSUFFICIENT_STOCK;
    }

    cart_elem_t *elem = find_cart_elem(cart, item->name); 

    if (elem) {
        elem->quant += quant;
    } 
    else {
        cart_elem_t *new_cart_item = calloc(1, sizeof(cart_elem_t));
        new_cart_item->name = strdup(item->name);
        new_cart_item->quant = quant;
        ioopm_list_append(cart->l, ptr_elem(new_cart_item));
    }

    item->total_reserved += quant;
    cart->total_cost += quant * item->price;
    return DB_OK;
}

db_status_t db_remove_from_cart(ioopm_hash_table_t *ht_n_i, cart_t *cart, char *name, int quant)
{
    // Hitta rätt rad i kundvagnen
    cart_elem_t *elem = find_cart_elem(cart, name);
    if (!elem)
    {
        return DB_NOT_IN_CART;
    }

    item_t *item = NULL;
    if (db_get_item(ht_n_i, name, &item) != DB_OK)
    {
        return DB_NO_SUCH_MERCH;
    }

    // samma som i kundvagnen om quant är större
    int removed = quant < elem->quant ? quant : elem->quant;

    elem->quant -= removed;
    item->total_reserved -= removed;
    cart->total_cost -= removed * item->price;

    if (elem->quant == 0)
    {
        ioopm_list_iterator_t *it = ioopm_list_iterator_create(cart->l);
        while (!ioopm_list_iterator_at_end(it))
        {
            if (ioopm_list_iterator_current(it).p == elem)
            {
                ioopm_list_iterator_remove(it);
                break;
            }
            ioopm_list_iterator_advance(it);
        }
        ioopm_list_iterator_destroy(it);

        free(elem->name);
        free(elem);
    }

    return DB_OK;
}

int db_checkout(cart_t *cart, ioopm_hash_table_t *ht_n_i, ioopm_hash_table_t *ht_sl_n)
{
    int total_cost = cart->total_cost;

    // For each item in the cart
    ioopm_list_iterator_t *cart_iter = ioopm_list_iterator_create(cart->l);
    while (!ioopm_list_iterator_at_end(cart_iter))
    {

        // Find the corresponding item in ht_n_i
        cart_elem_t *curr_cart = ioopm_list_iterator_current(cart_iter).p;
        item_t *item;
        db_get_item(ht_n_i, curr_cart->name, &item);

        // Set remaining quantity to the cart item's quantity
        int remaining_quantity = curr_cart->quant;

        // Iterate through the item's stock list
        ioopm_list_iterator_t *item_iter = ioopm_list_iterator_create(item->l);
        while(!ioopm_list_iterator_at_end(item_iter))
        {
            stock_entry_t *curr_stock = ioopm_list_iterator_current(item_iter).p;
            if (remaining_quantity == 0)
            {
                break;
            } else if (remaining_quantity >= curr_stock->quant)
            {
                // decrease remaining_quantity and total stock
                remaining_quantity -= curr_stock->quant;
                item->total_stock -= curr_stock->quant;

                // Remove the location from ht_sl_n
                elem_t removed_key;
                elem_t removed_value;
                ioopm_hash_table_remove(ht_sl_n, string_elem(curr_stock->sl), &removed_key, &removed_value);
                free(removed_key.s);
                free(removed_value.s);

                // Remove the stock entry from the item's list
                free(curr_stock->sl);
                free(curr_stock);
                ioopm_list_iterator_remove(item_iter);

            } else if (remaining_quantity < curr_stock->quant)
            {
            

                // decerase remaining_quantity from current location
                curr_stock->quant -= remaining_quantity;

                // Decrease total_stock by remaining quantity
                item->total_stock -= remaining_quantity;

                // done!
                remaining_quantity = 0;

            }
        }
        ioopm_list_iterator_destroy(item_iter);

        // Decrease total_reserved by the cart item's quantity
        item->total_reserved -= curr_cart->quant;
        ioopm_list_iterator_advance(cart_iter);
    }
    ioopm_list_iterator_destroy(cart_iter);
    // Empty the cart and free its elements
    db_destroy_cart(cart);
    return total_cost;
}
    
