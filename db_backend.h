#pragma once

#include "hash_table.h"
#include "linked_list.h"

typedef struct item item_t;
typedef struct cart cart_t;

/// @brief creates an item
/// @param name the name of the item
/// @param desc a desription of the item
/// @param price the price of the item in sek(?)
/// @returns a pointer to the newly created item
item_t *db_create_item(char *name, char *desc, int price);

/// @brief adds new merch to the warehous
/// @param ht_n_i a pointer to a ht mapping item name -> item*
/// @param merch a pointer to the item to insert
void db_add_merch(ioopm_hash_table_t *ht_n_i, item_t *merch);

/// @brief prints the name and location of all merch in the warehouse 20 at a time
/// @param ht_n_i a pointer to a ht mapping item name -> item*
void db_list_merch(ioopm_hash_table_t *ht_n_i);

/// @brief removes an item from the warehouse entirely, including all its stock
/// @param ht_n_i a pointer to a ht mapping item name -> item*
/// @param ht_sl_n a pointer to ht mapping storage location-> name of stored item
/// @param name name of the item to remove
void db_remove_merch(ioopm_hash_table_t *ht_n_i, ioopm_hash_table_t *ht_sl_n, char *name);

/// @brief change name, desc. or price of a merch, 
//         changing name of an existing merch not allowed
/// @param ht_n_i a pointer to a ht mapping item name -> item*
/// @param merch a pointer to a newly created merch item.
/// @note this function should be used in conjunction with a function that 
// creates an edited copy of an existing item, removes it and inserts a new one
void db_edit_merch(ioopm_hash_table_t *ht_n_i, item_t *merch);

/// @brief list all storage locations, quantity for a particular merch
/// @param name name of item to show stock for
void db_show_stock(item_t *item);

/// @brief increase the stock of a merch by at least one
/// @param ht_n_i a pointer to a ht mapping item name -> item*
/// @param ht_sl_n a pointer to ht mapping storage location -> name of stored item
/// @param item a pointer to the item to increase
/// @param quant the amount to increase quantity with
/// @param sl the name of the storage location in which to store in
/// @note we only replenish if the merch actually exists in the inventory
void db_replenish(ioopm_hash_table_t *ht_n_i, ioopm_hash_table_t *ht_sl_n, item_t *item, int quant, char *sl);

/// @brief creates an empty cart
/// @param no_carts a pointer to a variable keeping track of how many carts there are
/// @returns a pointer to the newly created cart
cart_t* db_create_cart(int *no_carts); 

/// @brief removes a cart from the system
/// @param id poitner to the cart to remove
void db_remove_cart(cart_t *cart);

/// @brief adds a quantity of an item to a cart
/// @param cart shopping cart to add item to
/// @param item item to add to shopping cart
/// @param quant number of item to add
void db_add_to_cart(cart_t *cart, item_t *item, int quant);

/// @brief removes zero or more items of some merch from a cart
/// @param cart cart to remove from
/// @param item item to remove
/// @param quant number to remove
void db_remove_from_cart(cart_t *cart, item_t *item, int quant);

/// @brief calculates the total cost of the cart
/// @param cart the cart to calculate for
int db_calculate_cost(cart_t *cart);

/// @brief decreases the stock of all items in the cart accoringly, removes cart from system
/// @param cart the cart to checkout
/// @param ht_sl_n a pointer to ht mapping storage location-> name of stored item
void db_checkout(cart_t *cart, ioopm_hash_table_t ht_sl_n);