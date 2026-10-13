#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct webstore webstore_t;

typedef enum
{
  WS_OK = 0,
  WS_MERCH_EXISTS,       ///< En vara med det namnet finns redan
  WS_NO_SUCH_MERCH,      ///< Varan finns inte
  WS_NO_SUCH_CART,       ///< Kundvagnen finns inte
  WS_LOCATION_TAKEN,     ///< Lagerplatsen innehåller redan en annan vara
  WS_INSUFFICIENT_STOCK, ///< Det finns inte tillräckligt i lager för alla kundvagnar
  WS_NOT_IN_CART,        ///< Varan finns inte i kundvagnen
} ws_status_t;

/// Information om en vara, för listning.
typedef struct
{
  const char *name; ///< Lånad pekare
  const char *desc; ///< Lånad pekare
  int price;        ///< Pris i hela SEK
} ws_merch_info_t;

/// En lagerplats och hur många exemplar som ligger där.
typedef struct
{
  const char *location; ///< Lånad pekare, t.ex. "A25"
  int quantity;         ///< Alltid >= 1
} ws_stock_entry_t;

/// @brief Skapar en tom databas.
webstore_t *ws_create(void);

/// @brief Frigör databasen och allt den äger.
void ws_destroy(webstore_t *ws);

/// @brief Lägger till en ny vara som inte finns i lager.
/// @return WS_OK eller WS_MERCH_EXISTS
ws_status_t ws_add_merch(webstore_t *ws, const char *name, const char *desc, int price);

/// @brief Hämtar alla varor sorterade i bokstavsordning på namn.
/// @param count fylls i med antalet varor
/// @return malloc:ad array som anroparen frigör med free() (men inte strängarna i den)
ws_merch_info_t *ws_list_merch(webstore_t *ws, size_t *count);

/// @brief Tar bort en vara helt, inklusive dess lager.
/// @return WS_OK eller WS_NO_SUCH_MERCH
ws_status_t ws_remove_merch(webstore_t *ws, const char *name);

/// @brief Ändrar namn, beskrivning och pris på en vara. Lagret påverkas inte.
/// @return WS_OK, WS_NO_SUCH_MERCH eller WS_MERCH_EXISTS (om new_name redan används av en annan vara)
ws_status_t ws_edit_merch(webstore_t *ws, const char *old_name,
                          const char *new_name, const char *new_desc, int new_price);

/// @brief Hämtar alla lagerplatser för en vara, sorterade i bokstavsordning.
/// @param count fylls i med antalet lagerplatser
/// @param status fylls i med WS_OK eller WS_NO_SUCH_MERCH
/// @return malloc:ad array som anroparen frigör med free(), eller NULL om count blir 0
ws_stock_entry_t *ws_show_stock(webstore_t *ws, const char *name, size_t *count, ws_status_t *status);

/// @brief Ökar lagret för en vara på en viss lagerplats.
/// @param amount antal exemplar, >= 1
/// @return WS_OK, WS_NO_SUCH_MERCH eller WS_LOCATION_TAKEN
ws_status_t ws_replenish(webstore_t *ws, const char *location, const char *name, int amount);

/// @brief Skapar en tom kundvagn.
/// @return kundvagnens id (1 för den första, 2 för den andra, osv.)
int ws_create_cart(webstore_t *ws);

/// @return WS_OK eller WS_NO_SUCH_CART
ws_status_t ws_remove_cart(webstore_t *ws, int cart_id);

/// @return WS_OK, WS_NO_SUCH_CART, WS_NO_SUCH_MERCH eller WS_INSUFFICIENT_STOCK
ws_status_t ws_add_to_cart(webstore_t *ws, int cart_id, const char *name, int amount);

/// @brief Tar bort amount (>= 0) exemplar av en vara ur kundvagnen.
/// @return WS_OK, WS_NO_SUCH_CART, WS_NO_SUCH_MERCH eller WS_NOT_IN_CART
ws_status_t ws_remove_from_cart(webstore_t *ws, int cart_id, const char *name, int amount);

/// @param cost fylls i med totalkostnaden om WS_OK returneras
/// @return WS_OK eller WS_NO_SUCH_CART
ws_status_t ws_cart_cost(webstore_t *ws, int cart_id, int *cost);

/// @brief Minskar lagret för varorna i kundvagnen och tar bort kundvagnen.
/// @return WS_OK eller WS_NO_SUCH_CART
ws_status_t ws_checkout(webstore_t *ws, int cart_id);
