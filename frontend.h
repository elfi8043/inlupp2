#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct webstore webstore_t;

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

/// En lagerplats och hur många exemplar som ligger där.
typedef struct
{
  const char *location; ///< Lånad pekare, t.ex. "A25"
  int quantity;         ///< Alltid >= 1
} db_stock_entry_t;

/// @brief Skapar en tom databas.
webstore_t *db_create(void);

/// @brief Frigör databasen och allt den äger.
void db_destroy(webstore_t *db);

/// @brief Lägger till en ny vara som inte finns i lager.
/// @return DB_OK eller DB_MERCH_EXISTS
db_status_t db_add_merch(webstore_t *db, const char *name, const char *desc, int price);

/// @brief Hämtar alla varor sorterade i bokstavsordning på namn.
/// @param count fylls i med antalet varor
/// @return malloc:ad array som anroparen frigör med free() (men inte strängarna i den)
db_merch_info_t *db_list_merch(webstore_t *db, size_t *count);

/// @brief Tar bort en vara helt, inklusive dess lager.
/// @return DB_OK eller DB_NO_SUCH_MERCH
db_status_t db_remove_merch(webstore_t *db, const char *name);

/// @brief Ändrar namn, beskrivning och pris på en vara. Lagret påverkas inte.
/// @return DB_OK, DB_NO_SUCH_MERCH eller DB_MERCH_EXISTS (om new_name redan används av en annan vara)
db_status_t db_edit_merch(webstore_t *db, const char *old_name,
                          const char *new_name, const char *new_desc, int new_price);

/// @brief Hämtar alla lagerplatser för en vara, sorterade i bokstavsordning.
/// @param count fylls i med antalet lagerplatser
/// @param status fylls i med DB_OK eller DB_NO_SUCH_MERCH
/// @return malloc:ad array som anroparen frigör med free(), eller NULL om count blir 0
db_stock_entry_t *db_show_stock(webstore_t *db, const char *name, size_t *count, db_status_t *status);

/// @brief Ökar lagret för en vara på en viss lagerplats.
/// @param amount antal exemplar, >= 1
/// @return DB_OK, DB_NO_SUCH_MERCH eller DB_LOCATION_TAKEN
db_status_t db_replenish(webstore_t *db, const char *location, const char *name, int amount);

/// @brief Skapar en tom kundvagn.
/// @return kundvagnens id (1 för den första, 2 för den andra, osv.)
int db_create_cart(webstore_t *db);

/// @return DB_OK eller DB_NO_SUCH_CART
db_status_t db_remove_cart(webstore_t *db, int cart_id);

/// @return DB_OK, DB_NO_SUCH_CART, DB_NO_SUCH_MERCH eller DB_INSUFFICIENT_STOCK
db_status_t db_add_to_cart(webstore_t *db, int cart_id, const char *name, int amount);

/// @brief Tar bort amount (>= 0) exemplar av en vara ur kundvagnen.
/// @return DB_OK, DB_NO_SUCH_CART, DB_NO_SUCH_MERCH eller DB_NOT_IN_CART
db_status_t db_remove_from_cart(webstore_t *db, int cart_id, const char *name, int amount);

/// @param cost fylls i med totalkostnaden om DB_OK returneras
/// @return DB_OK eller DB_NO_SUCH_CART
db_status_t db_cart_cost(webstore_t *db, int cart_id, int *cost);

/// @brief Minskar lagret för varorna i kundvagnen och tar bort kundvagnen.
/// @return DB_OK eller DB_NO_SUCH_CART
db_status_t db_checkout(webstore_t *db, int cart_id);
