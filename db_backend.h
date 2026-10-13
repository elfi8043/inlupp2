#pragma once
#include "hash_table.h"
#include "linked_list.h"

typedef enum
{
  DB_OK = 0,
  DB_MERCH_EXISTS,       ///< En vara med det namnet finns redan
  DB_NO_SUCH_MERCH,      ///< Varan finns inte
  DB_NO_SUCH_CART,       ///< Kundvagnen finns inte
  DB_LOCATION_TAKEN,     ///< Lagerplatsen innehåller redan en annan vara
  DB_INSUFFICIENT_STOCK, ///< Det finns inte tillräckligt i lager för alla kundvagnar
  DB_NOT_IN_CART,        ///< Varan finns inte i kundvagnen
} db_status_t;

// stores where item is stored and quantity at that location
// for use in item->l
typedef struct
{
    char *sl;
    int quant;
    int no_reserved; // number of items of this merch reserved in a cart
} stock_entry_t;

typedef struct
{
    char *name;
    char *desc;
    int price;
    int total_stock;
    int total_reserved;
    ioopm_list_t *l;
} item_t;

typedef struct
{
    ioopm_list_t *l;
    int id;
    int total_cost; //add to this number everytime we add an item
} cart_t;

typedef struct
{
    char *name;
    int quant;
} cart_elem_t;

/// @brief creates an item
/// @param name the name of the item
/// @param desc a desription of the item
/// @param price the price of the item in sek(?)
/// @returns a pointer to the newly created item
item_t *db_create_item(char *name, char *desc, int price);

/// @brief adds new merch to the warehous
/// @param ht_n_i a pointer to a ht mapping item name -> item*
/// @param merch a pointer to the item to insert
db_status_t db_add_merch(ioopm_hash_table_t *ht_n_i, item_t *merch);

/// @brief prints the name and location of all merch in the warehouse 20 at a time
/// @param ht_n_i a pointer to a ht mapping item name -> item*
db_status_t db_list_merch(ioopm_hash_table_t *ht_n_i);

/// @brief removes an item from the warehouse entirely, including all its stock
/// @param ht_n_i a pointer to a ht mapping item name -> item*
/// @param ht_sl_n a pointer to ht mapping storage location-> name of stored item
/// @param name name of the item to remove
db_status_t db_remove_merch(ioopm_hash_table_t *ht_n_i, ioopm_hash_table_t *ht_sl_n, char *name);

/// @brief change name, desc. or price of a merch, 
//         changing name of an existing merch not allowed
/// @param ht_n_i a pointer to a ht mapping item name -> item*
/// @param merch a pointer to a newly created merch item.
/// @note this function should be used in conjunction with a function that 
// creates an edited copy of an existing item, removes it and inserts a new one
db_status_t db_edit_merch(ioopm_hash_table_t *ht_n_i, item_t *merch);

/// @brief list all storage locations, quantity for a particular merch
/// @param name name of item to show stock for
db_status_t db_show_stock(item_t *item);

/// @brief increase the stock of a merch by at least one
/// @param ht_n_i a pointer to a ht mapping item name -> item*
/// @param ht_sl_n a pointer to ht mapping storage location -> name of stored item
/// @param item a pointer to the item to increase
/// @param quant the amount to increase quantity with
/// @param sl the name of the storage location in which to store in
/// @note we only replenish if the merch actually exists in the inventory
db_status_t db_replenish(ioopm_hash_table_t *ht_n_i, ioopm_hash_table_t *ht_sl_n, item_t *item, int quant, char *sl);

/// @brief creates an empty cart
/// @param no_carts a pointer to a variable keeping track of how many carts there are
/// @returns a pointer to the newly created cart
int db_create_cart(int *no_carts); 

/// @brief removes a cart from the system
/// @param id poitner to the cart to remove
db_status_t db_remove_cart(cart_t *cart);

/// @brief adds a quantity of an item to a cart
/// @param cart shopping cart to add item to
/// @param item item to add to shopping cart
/// @param quant number of item to add
db_status_t db_add_to_cart(cart_t *cart, item_t *item, int quant);

/// @brief removes zero or more items of some merch from a cart
/// @param cart cart to remove from
/// @param item item to remove
/// @param quant number to remove
db_status_t db_remove_from_cart(cart_t *cart, item_t *item, int quant);

/// @brief calculates the total cost of the cart
/// @param cart the cart to calculate for
db_status_t db_calculate_cost(cart_t *cart);

/// @brief decreases the stock of all items in the cart accoringly, removes cart from system
/// @param cart the cart to checkout
/// @param ht_sl_n a pointer to ht mapping storage location-> name of stored item
db_status_t db_checkout(cart_t *cart, ioopm_hash_table_t ht_sl_n);