#pragma once
#include "hash_table.h"
#include "linked_list.h"
#include "hash_table.h"
#include "hash_table_iterator.h"
#include "list_iterator.h"
#include "utils.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
  DB_OK = 0,
  DB_MERCH_EXISTS,       // En vara med det namnet finns redan
  DB_NO_SUCH_MERCH,      // Varan finns inte
  DB_NO_SUCH_CART,       // Kundvagnen finns inte
  DB_LOCATION_TAKEN,     // Lagerplatsen innehåller redan en annan vara
  DB_INSUFFICIENT_STOCK, // Det finns inte tillräckligt i lager för alla kundvagnar
  DB_NOT_IN_CART,        // Varan finns inte i kundvagnen
  DB_MERCH_IN_CART,      // Varan finns i en kundvagn
} db_status_t;

// stores where item is stored and quantity at that location
// for use in item->l
typedef struct
{
    char *sl;
    int quant;
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

/// En kundvagn. Innehållet är dolt; använd funktionerna nedan.
typedef struct cart cart_t;

/// Funktionstyp för db_cart_apply: anropas en gång per rad i kundvagnen.
typedef void cart_row_fn(char *name, int quant);


/**
 * @brief Creates a new item.
 * The strings are duplicated, and an empty stock list is created.
 *
 * @param name Name of the item.
 * @param desc Description of the item.
 * @param price Price of the item.
 * @return Pointer to the newly created item.
 */
item_t *db_create_item(char *name, char *desc, int price);


/**
 * @brief Adds a new item to the warehouse.
 * If an item with the same name already exists, the supplied item
 * is destroyed.
 *
 * @param ht_n_i Hash table mapping item names to item pointers.
 * @param merch Pointer to the item to insert.
 * @return DB_OK if successful, DB_MERCH_EXISTS if an item with
 *         the same name already exists.
 */
db_status_t db_add_merch(ioopm_hash_table_t *ht_n_i, item_t *merch);


/**
 * @brief Retrieves an item from the warehouse.
 * Looks up an item by name and stores its pointer in result.
 *
 * @param ht_n_i Hash table mapping item names to item pointers.
 * @param name Name of the item to retrieve.
 * @param result Output parameter receiving the item pointer.
 * @return DB_OK if the item was found, DB_NO_SUCH_MERCH otherwise.
 */
db_status_t db_get_item(ioopm_hash_table_t *ht_n_i, char *name, item_t **result);


/**
 * @brief Removes an item from the warehouse.
 * Removes the item from both hash tables, all its associated
 * stock entries. The item's allocated memory is frerd.
 * An item cannot be removed while it is reserved in a shopping cart.
 *
 * @param ht_n_i Hash table mapping item names to item pointers.
 * @param ht_sl_n Hash table mapping storage locations to item names.
 * @param name Name of the item to remove.
 * @return DB_OK if successful, DB_NO_SUCH_MERCH if the item does not
 *         exist, or DB_MERCH_IN_CART if the item is reserved in a cart.
 */
db_status_t db_remove_merch(ioopm_hash_table_t *ht_n_i,
                            ioopm_hash_table_t *ht_sl_n,
                            char *name);


/**
 * @brief Edits an existing item's name, description and price.
 *
 * Creates a new replacement item for an existing item in order to edit it
 * The old item is removed and the replacement is inserted
 *
 * Renaming is not permitted if another item already has the requested
 * name, or if the item is currently reserved in a shopping cart.
 *
 * @param ht_n_i Hash table mapping item names to item pointers.
 * @param ht_sl_n Hash table mapping storage locations to item names.
 * @param old_name Current name of the item.
 * @param name New name of the item.
 * @param desc New description of the item.
 * @param price New price of the item.
 * @return DB_OK if successful, DB_NO_SUCH_MERCH if the original item
 *         does not exist, DB_MERCH_EXISTS if the new name is already
 *         taken, or DB_MERCH_IN_CART if renaming a reserved item.
 */
db_status_t db_edit_merch(ioopm_hash_table_t *ht_n_i,
                          ioopm_hash_table_t *ht_sl_n,
                          char *old_name,
                          char *name,
                          char *desc,
                          int price);

/// @brief Hämtar en kundvagns id
int db_cart_id(cart_t *cart);

/// @brief Hämtar kundvagnens totala kostnad
int db_cart_cost(cart_t *cart);

/// @brief Antal olika varor i kundvagnen
size_t db_cart_size(cart_t *cart);

/// @brief Hur många av en vara kundvagnen innehåller
int db_cart_quantity(cart_t *cart, char *name);

/// @brief Anropar fn en gång för varje rad i kundvagnen
/// @param cart
/// @param fn function som ska använda cart element
void db_cart_apply(cart_t *cart, cart_row_fn *fn);


/// @brief destroys a carts memory
/// @param cart poitner to the cart to remove
void db_destroy_cart(cart_t *cart);




/**
 * @brief Removes a shopping cart and releases its stock reservations.
 * @param ht_n_i Hash table mapping item names to item pointers.
 * @param cart Pointer to the cart to remove.
 */
void db_remove_cart(ioopm_hash_table_t *ht_n_i, cart_t *cart);


/**
 * @brief Adds an item and a specified quantity to a shopping cart.
 * The item's reserved quantity and the cart's total cost are updated
 *
 * @param cart Cart to add the item to.
 * @param item Pointer to the item to add.
 * @param quant Quantity to add.
 * @return DB_OK if successful, or DB_INSUFFICIENT_STOCK if there is
 *         insufficient unreserved stock.
 */
db_status_t db_add_to_cart(cart_t *cart, item_t *item, int quant);


/**
 * @brief Removes an item or part of its quantity from a shopping cart.
 * Updates the item's reserved quantity and the cart's total cost.
 * If the quantity reaches zero, the cart entry is removed.
 * If stock quantity reaches zero the mapping in ht_sl_n is remove
 * 
 * @param ht_n_i Hash table mapping item names to item pointers.
 * @param cart Cart to remove the item from.
 * @param name Name of the item to remove.
 * @param quant Quantity to remove.
 * @return DB_OK if successful, DB_NOT_IN_CART if the item is not in
 *         the cart, or DB_NO_SUCH_MERCH if the item does not exist.
 */
db_status_t db_remove_from_cart(ioopm_hash_table_t *ht_n_i,
                                cart_t *cart,
                                char *name,
                                int quant);


/**
 * @brief Calculates the total cost of a shopping cart.
 *
 * @param cart Cart whose total cost is to be calculated.
 * @return Total cost of all items in the cart.
 */
int db_calculate_cost(cart_t *cart);


/**
 * @brief Checks out a shopping cart.
 * Removes the purchased quantities from the warehouse stock, consuming
 * stock from the item's storage locations until the requested quantity
 * has been fulfilled.
 *
 * Updates the total stock and reserved quantities, removes emptied
 * storage locations from the storage location hash table, and destroys
 * the cart.
 *
 * @param cart Cart to check out.
 * @param ht_n_i Hash table mapping item names to item pointers.
 * @param ht_sl_n Hash table mapping storage locations to item names.
 * @return Total cost of the completed purchase.
 */
int db_checkout(cart_t *cart,
                ioopm_hash_table_t *ht_n_i,
                ioopm_hash_table_t *ht_sl_n);


/**
 * @brief Destroys both warehouse hash tables and their contents.
 *
 * Frees all items, their associated stock entries, and all allocated
 * keys and values in both hash tables. Finally destroys the hash tables.
 *
 * @param ht_n_i Hash table mapping item names to item pointers.
 * @param ht_sl_n Hash table mapping storage locations to item names.
 */
void db_ht_destroy(ioopm_hash_table_t *ht_n_i,
                   ioopm_hash_table_t *ht_sl_n);