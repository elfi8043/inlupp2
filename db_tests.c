#include <CUnit/Basic.h>
#include "db_backend.h"
#include "hash_table_iterator.h"
#include "hash_table.h"
#include "linked_list.h"
#include "list_iterator.h"

#include <stdlib.h>
#include <string.h>

int init_suite(void)
{
    return 0;
}

int clean_suite(void)
{
    return 0;
}


/**
 * Lägger till 25 testvaror i databasen.
 */
void add_sample_merch_to_db(ioopm_hash_table_t *ht_n_i)
{
    char *names[25] = {
        "Jonkanon", "Dunderboj", "Skrammelmina", "Drakskold",
        "Blixtsvard", "Jarnhammare", "Trollstav", "Isyxa",
        "Eldlans", "Skogsdolk", "Stormbage", "Kopparskold",
        "Runsten", "Silverring", "Guldhalsband", "Laderhuva",
        "Jarnrustning", "Reseryggsack", "Faltkompass", "Rep",
        "Lykta", "Tandstal", "Vattenflaska", "Matkittel",
        "Lagerfilt"
    };

    char *descriptions[25] = {
        "Kanon som skjuter jon",
        "Explosiv boj for sjostrid",
        "Mina som skramlar vid kontakt",
        "Skold tillverkad av drakfjall",
        "Svärd som kanaliserar blixtar",
        "Tung hammare av jarn",
        "Stav med magiska runor",
        "Yxa som aldrig slutar vara kall",
        "Lans med en brinnande spets",
        "Liten dolk for skogsbruk",
        "Bage som skjuter med stormens kraft",
        "Skold av hamrad koppar",
        "Sten med urgamla runor",
        "Ring av rent silver",
        "Halsband av polerat guld",
        "Huva av slitstarkt lader",
        "Rustning av smitt jarn",
        "Ryggsack for langa resor",
        "Kompass for navigering i falt",
        "Starkt rep for klattring",
        "Lykta for morka natter",
        "Verktyg for att gora upp eld",
        "Flaska for att forvara vatten",
        "Kittel for matlagning over oppen eld",
        "Filt for kalla natter i lager"
    };

    int prices[25] = {
        15000, 8500, 3200, 12000, 9500,
        4500, 7000, 2800, 5500, 1800,
        6500, 3500, 2200, 1800, 4200,
        1200, 18000, 900, 750, 300,
        650, 250, 200, 1100, 500
    };

    for (int i = 0; i < 25; i++)
    {
        db_add_merch(
            ht_n_i,
            db_create_item(names[i], descriptions[i], prices[i])
        );
    }
}


/**
 * Skapar två tomma hashtabeller.
 */
static void create_test_tables(ioopm_hash_table_t **ht_n_i,
                               ioopm_hash_table_t **ht_sl_n)
{
    *ht_n_i = ioopm_hash_table_create(
        ioopm_hash_string, ioopm_string_comp
    );

    *ht_sl_n = ioopm_hash_table_create(
        ioopm_hash_string, ioopm_string_comp
    );
}

static item_t *add_item_in_stock(ioopm_hash_table_t *ht_n_i,
                                 ioopm_hash_table_t *ht_sl_n,
                                 char *name, int price,
                                 int quant, char *sl)
{
    db_add_merch(ht_n_i, db_create_item(name, "Testvara", price));
    db_replenish(ht_n_i, ht_sl_n, name, quant, sl);
 
    item_t *item = NULL;
    db_get_item(ht_n_i, name, &item);
    return item;
}

/**
 * Testar att en vara kan läggas till och att dubbletter avvisas.
 */
void test_add_merch(void)
{
    ioopm_hash_table_t *ht_n_i;
    ioopm_hash_table_t *ht_sl_n;
    create_test_tables(&ht_n_i, &ht_sl_n);

    char *name = "Jonkanon";
    char *desc = "Kanon som skjuter jon";

    item_t *item = db_create_item(name, desc, 100);

    CU_ASSERT_EQUAL(
        db_add_merch(ht_n_i, item),
        DB_OK
    );

    CU_ASSERT_TRUE(
        ioopm_hash_table_has_key(ht_n_i, string_elem(name))
    );

    // Skapa en separat vara för dubblettestet.
    // db_add_merch frigör varan när namnet redan finns.
    item_t *duplicate = db_create_item(name, desc, 100);

    CU_ASSERT_EQUAL(
        db_add_merch(ht_n_i, duplicate),
        DB_MERCH_EXISTS
    );

    db_ht_destroy(ht_n_i, ht_sl_n);
}


/**
 * Testar att lager kan fyllas på och att lagerplatsen sparas.
 */
void test_replenish(void)
{
    ioopm_hash_table_t *ht_n_i;
    ioopm_hash_table_t *ht_sl_n;
    create_test_tables(&ht_n_i, &ht_sl_n);

    add_sample_merch_to_db(ht_n_i);

    CU_ASSERT_EQUAL(
        db_replenish(ht_n_i, ht_sl_n, "Runsten", 10, "a10"),
        DB_OK
    );

    // Kontrollera att lagerplatsen finns i lagerplatstabellen.
    elem_t res;

    CU_ASSERT_TRUE(
        ioopm_hash_table_lookup(ht_sl_n, string_elem("a10"), &res)
    );

    CU_ASSERT_STRING_EQUAL(res.s, "Runsten");

    // Kontrollera att varans lagerlista innehåller rätt lagerplats.
    item_t *item = NULL;

    CU_ASSERT_EQUAL(
        db_get_item(ht_n_i, "Runsten", &item),
        DB_OK
    );

    stock_entry_t *stock = ioopm_list_head(item->l).p;

    CU_ASSERT_STRING_EQUAL(stock->sl, "a10");
    CU_ASSERT_EQUAL(stock->quant, 10);

    db_ht_destroy(ht_n_i, ht_sl_n);
}


/**
 * Kontrollerar att databasen behåller sina egna kopior av
 * varans namn och beskrivning när anroparen frigör sina strängar.
 */
void test_add_merch_survives_caller_free(void)
{
    ioopm_hash_table_t *ht_n_i;
    ioopm_hash_table_t *ht_sl_n;
    create_test_tables(&ht_n_i, &ht_sl_n);

    char *name = strdup("Bralla");
    char *desc = strdup("Balla");

    item_t *item = db_create_item(name, desc, 20);

    CU_ASSERT_EQUAL(
        db_add_merch(ht_n_i, item),
        DB_OK
    );

    // Simulera att anroparen frigör sina egna strängar.
    free(name);
    free(desc);

    // Allokera annan data för att inte förlita oss på att
    // frigjort minne fortfarande innehåller samma bytes.
    char *junk = strdup("XXXXXX");

    item_t *found = NULL;

    CU_ASSERT_EQUAL(
        db_get_item(ht_n_i, "Bralla", &found),
        DB_OK
    );

    CU_ASSERT_PTR_EQUAL(found, item);
    CU_ASSERT_STRING_EQUAL(found->name, "Bralla");
    CU_ASSERT_STRING_EQUAL(found->desc, "Balla");

    free(junk);

    db_ht_destroy(ht_n_i, ht_sl_n);
}


/**
 * Testar att en vara kan byta namn och att dess lager
 * återskapas med rätt lagerplats och antal.
 */
void test_edit_merch(void)
{
    ioopm_hash_table_t *ht_n_i;
    ioopm_hash_table_t *ht_sl_n;
    create_test_tables(&ht_n_i, &ht_sl_n);

    add_sample_merch_to_db(ht_n_i);

    // Lägg till lager före namnändringen.
    CU_ASSERT_EQUAL(
        db_replenish(ht_n_i, ht_sl_n, "Runsten", 10, "a10"),
        DB_OK
    );

    CU_ASSERT_EQUAL(
        db_replenish(ht_n_i, ht_sl_n, "Runsten", 5, "b10"),
        DB_OK
    );

    // Ändra namn, beskrivning och pris.
    CU_ASSERT_EQUAL(
        db_edit_merch(
            ht_n_i, ht_sl_n,
            "Runsten", "kulsten", "rolig sten", 40
        ),
        DB_OK
    );

    item_t *item = NULL;

    // Det gamla namnet ska inte längre finnas.
    CU_ASSERT_EQUAL(
        db_get_item(ht_n_i, "Runsten", &item),
        DB_NO_SUCH_MERCH
    );

    // Det nya namnet ska finnas med uppdaterade uppgifter.
    CU_ASSERT_EQUAL(
        db_get_item(ht_n_i, "kulsten", &item),
        DB_OK
    );

    CU_ASSERT_STRING_EQUAL(item->name, "kulsten");
    CU_ASSERT_STRING_EQUAL(item->desc, "rolig sten");
    CU_ASSERT_EQUAL(item->price, 40);

    // Kontrollera att båda lagerposterna har bevarats.
    CU_ASSERT_EQUAL(ioopm_list_size(item->l), 2);

    bool found_a10 = false;
    bool found_b10 = false;

    ioopm_list_iterator_t *it =
        ioopm_list_iterator_create(item->l);

    while (!ioopm_list_iterator_at_end(it))
    {
        stock_entry_t *stock = ioopm_list_iterator_current(it).p;

        if (strcmp(stock->sl, "a10") == 0)
        {
            found_a10 = true;
            CU_ASSERT_EQUAL(stock->quant, 10);
        }
        else if (strcmp(stock->sl, "b10") == 0)
        {
            found_b10 = true;
            CU_ASSERT_EQUAL(stock->quant, 5);
        }

        ioopm_list_iterator_advance(it);
    }

    ioopm_list_iterator_destroy(it);

    CU_ASSERT_TRUE(found_a10);
    CU_ASSERT_TRUE(found_b10);

    // Lagerplatstabellen ska nu koppla båda platserna till
    // den nya varan, inte till det gamla namnet.
    elem_t res;

    CU_ASSERT_TRUE(
        ioopm_hash_table_lookup(ht_sl_n, string_elem("a10"), &res)
    );
    CU_ASSERT_STRING_EQUAL(res.s, "kulsten");

    CU_ASSERT_TRUE(
        ioopm_hash_table_lookup(ht_sl_n, string_elem("b10"), &res)
    );
    CU_ASSERT_STRING_EQUAL(res.s, "kulsten");

    db_ht_destroy(ht_n_i, ht_sl_n);
}


/**
 * Testar att det går att ta bort en vara och dess lagerplatser.
 */
void test_remove_merch(void)
{
    ioopm_hash_table_t *ht_n_i;
    ioopm_hash_table_t *ht_sl_n;
    create_test_tables(&ht_n_i, &ht_sl_n);

    add_sample_merch_to_db(ht_n_i);

    CU_ASSERT_EQUAL(
        db_replenish(ht_n_i, ht_sl_n, "Runsten", 10, "a10"),
        DB_OK
    );

    CU_ASSERT_EQUAL(
        db_remove_merch(ht_n_i, ht_sl_n, "Runsten"),
        DB_OK
    );

    item_t *item = NULL;
    elem_t res;

    CU_ASSERT_EQUAL(
        db_get_item(ht_n_i, "Runsten", &item),
        DB_NO_SUCH_MERCH
    );

    CU_ASSERT_FALSE(
        ioopm_hash_table_lookup(ht_sl_n, string_elem("a10"), &res)
    );

    db_ht_destroy(ht_n_i, ht_sl_n);
}

void test_create_cart(void)
{
    int no_carts = 0;
 
    cart_t *c1 = db_create_cart(&no_carts);
    cart_t *c2 = db_create_cart(&no_carts);
 
    CU_ASSERT_EQUAL(c1->id, 1);
    CU_ASSERT_EQUAL(c2->id, 2);
    CU_ASSERT_EQUAL(no_carts, 2);
    CU_ASSERT_TRUE(ioopm_list_is_empty(c1->l));
    CU_ASSERT_EQUAL(c1->total_cost, 0);
 
    db_destroy_cart(c1);
    db_destroy_cart(c2);
}

void test_add_same_item_twice(void)
{
    ioopm_hash_table_t *ht_n_i;
    ioopm_hash_table_t *ht_sl_n;
    create_test_tables(&ht_n_i, &ht_sl_n);
 
    item_t *item = add_item_in_stock(ht_n_i, ht_sl_n, "Runsten", 50, 10, "A10");
 
    int no_carts = 0;
    cart_t *cart = db_create_cart(&no_carts);
    db_add_to_cart(cart, item, 2);
    db_add_to_cart(cart, item, 3);
 
    CU_ASSERT_EQUAL(ioopm_list_size(cart->l), 1);
    cart_elem_t *elem = find_cart_elem(cart, "Runsten");
    CU_ASSERT_PTR_NOT_NULL(elem);
    if (elem)
    {
        CU_ASSERT_EQUAL(elem->quant, 5);
    }
    CU_ASSERT_EQUAL(item->total_reserved, 5);
    CU_ASSERT_EQUAL(cart->total_cost, 5 * 50);
 
    db_remove_cart(ht_n_i, cart);
    db_ht_destroy(ht_n_i, ht_sl_n);
}

void test_add_to_cart_insufficient_stock(void)
{
    ioopm_hash_table_t *ht_n_i;
    ioopm_hash_table_t *ht_sl_n;
    create_test_tables(&ht_n_i, &ht_sl_n);
 
    item_t *item = add_item_in_stock(ht_n_i, ht_sl_n, "Runsten", 50, 5, "A10");
 
    int no_carts = 0;
    cart_t *cart = db_create_cart(&no_carts);
 
    CU_ASSERT_EQUAL(db_add_to_cart(cart, item, 6), DB_INSUFFICIENT_STOCK);
    CU_ASSERT_TRUE(ioopm_list_is_empty(cart->l));
    CU_ASSERT_EQUAL(item->total_reserved, 0);
 
    // Exakt hela lagret ska gå bra.
    CU_ASSERT_EQUAL(db_add_to_cart(cart, item, 5), DB_OK);
 
    db_remove_cart(ht_n_i, cart);
    db_ht_destroy(ht_n_i, ht_sl_n);
}
 
void test_remove_from_cart_not_in_cart(void)
{
    ioopm_hash_table_t *ht_n_i;
    ioopm_hash_table_t *ht_sl_n;
    create_test_tables(&ht_n_i, &ht_sl_n);
 
    add_item_in_stock(ht_n_i, ht_sl_n, "Runsten", 50, 10, "A10");
 
    int no_carts = 0;
    cart_t *cart = db_create_cart(&no_carts);
 
    CU_ASSERT_EQUAL(db_remove_from_cart(ht_n_i, cart, "Runsten", 1), DB_NOT_IN_CART);
 
    db_remove_cart(ht_n_i, cart);
    db_ht_destroy(ht_n_i, ht_sl_n);
}

void test_remove_cart_releases_reservation(void)
{
    ioopm_hash_table_t *ht_n_i;
    ioopm_hash_table_t *ht_sl_n;
    create_test_tables(&ht_n_i, &ht_sl_n);
 
    item_t *item = add_item_in_stock(ht_n_i, ht_sl_n, "Runsten", 50, 5, "A10");
 
    int no_carts = 0;
    cart_t *c1 = db_create_cart(&no_carts);
    db_add_to_cart(c1, item, 5);
 
    db_remove_cart(ht_n_i, c1);
    CU_ASSERT_EQUAL(item->total_reserved, 0);
 
    // Hela lagret ska nu gå att lägga i en ny kundvagn.
    cart_t *c2 = db_create_cart(&no_carts);
    CU_ASSERT_EQUAL(db_add_to_cart(c2, item, 5), DB_OK);
 
    db_remove_cart(ht_n_i, c2);
    db_ht_destroy(ht_n_i, ht_sl_n);
}

void test_checkout_single_location(void)
{
    ioopm_hash_table_t *ht_n_i;
    ioopm_hash_table_t *ht_sl_n;
    create_test_tables(&ht_n_i, &ht_sl_n);
 
    item_t *item = add_item_in_stock(ht_n_i, ht_sl_n, "Runsten", 50, 10, "A10");
 
    int no_carts = 0;
    cart_t *cart = db_create_cart(&no_carts);
    db_add_to_cart(cart, item, 4);
 
    // Checkout returnerar kostnaden och frigör kundvagnen.
    CU_ASSERT_EQUAL(db_checkout(cart, ht_n_i, ht_sl_n), 4 * 50);
 
    CU_ASSERT_EQUAL(item->total_stock, 6);
    CU_ASSERT_EQUAL(item->total_reserved, 0);
 
    stock_entry_t *stock = ioopm_list_head(item->l).p;
    CU_ASSERT_EQUAL(stock->quant, 6);
 
    elem_t res;
    CU_ASSERT_TRUE(ioopm_hash_table_lookup(ht_sl_n, string_elem("A10"), &res));
 
    db_ht_destroy(ht_n_i, ht_sl_n);
}
 
 
/**
 * Testar checkout som tömmer en lagerplats och tar resten från nästa.
 */
void test_checkout_empties_location(void)
{
    ioopm_hash_table_t *ht_n_i;
    ioopm_hash_table_t *ht_sl_n;
    create_test_tables(&ht_n_i, &ht_sl_n);
 
    item_t *item = add_item_in_stock(ht_n_i, ht_sl_n, "Runsten", 50, 4, "A10");
    db_replenish(ht_n_i, ht_sl_n, "Runsten", 6, "B10");
 
    int no_carts = 0;
    cart_t *cart = db_create_cart(&no_carts);
    db_add_to_cart(cart, item, 5);
 
    db_checkout(cart, ht_n_i, ht_sl_n);
 
    CU_ASSERT_EQUAL(item->total_stock, 5);
    CU_ASSERT_EQUAL(item->total_reserved, 0);
 
    // A10 tömdes och ska vara borta överallt.
    CU_ASSERT_EQUAL(ioopm_list_size(item->l), 1);
    elem_t res;
    CU_ASSERT_FALSE(ioopm_hash_table_lookup(ht_sl_n, string_elem("A10"), &res));
 
    stock_entry_t *stock = ioopm_list_head(item->l).p;
    CU_ASSERT_STRING_EQUAL(stock->sl, "B10");
    CU_ASSERT_EQUAL(stock->quant, 5);
 
    // Varan ska fortfarande gå att hitta på sitt namn.
    item_t *found = NULL;
    CU_ASSERT_EQUAL(db_get_item(ht_n_i, "Runsten", &found), DB_OK);
    CU_ASSERT_STRING_EQUAL(item->name, "Runsten");
 
    db_ht_destroy(ht_n_i, ht_sl_n);
}
 
 
/**
 * Testar checkout med flera olika varor i samma kundvagn.
 */
void test_checkout_multiple_items(void)
{
    ioopm_hash_table_t *ht_n_i;
    ioopm_hash_table_t *ht_sl_n;
    create_test_tables(&ht_n_i, &ht_sl_n);
 
    item_t *first = add_item_in_stock(ht_n_i, ht_sl_n, "Runsten", 50, 10, "A10");
    item_t *second = add_item_in_stock(ht_n_i, ht_sl_n, "Lykta", 20, 3, "B10");
 
    int no_carts = 0;
    cart_t *cart = db_create_cart(&no_carts);
    db_add_to_cart(cart, first, 2);
    db_add_to_cart(cart, second, 3);
 
    CU_ASSERT_EQUAL(db_checkout(cart, ht_n_i, ht_sl_n), 2 * 50 + 3 * 20);
 
    CU_ASSERT_EQUAL(first->total_stock, 8);
    CU_ASSERT_EQUAL(second->total_stock, 0);
 
    // Lykta har inget lager kvar, så B10 ska vara ledig.
    CU_ASSERT_TRUE(ioopm_list_is_empty(second->l));
    elem_t res;
    CU_ASSERT_FALSE(ioopm_hash_table_lookup(ht_sl_n, string_elem("B10"), &res));
 
    db_ht_destroy(ht_n_i, ht_sl_n);
}

int main(void)
{
    if (CU_initialize_registry() != CUE_SUCCESS)
    {
        return CU_get_error();
    }

    CU_pSuite my_test_suite = CU_add_suite(
        "DB backend tests",
        init_suite,
        clean_suite
    );

    if (my_test_suite == NULL)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // Registrera samtliga tester.
    if (
        CU_add_test(my_test_suite, "add merch", test_add_merch) == NULL ||
        CU_add_test(my_test_suite, "replenish", test_replenish) == NULL ||
        CU_add_test(my_test_suite, "caller frees strings",
                    test_add_merch_survives_caller_free) == NULL ||
        CU_add_test(my_test_suite, "edit merch", test_edit_merch) == NULL ||
        CU_add_test(my_test_suite, "remove merch", test_remove_merch) == NULL ||
        CU_add_test(my_test_suite, "create cart", test_create_cart) == NULL ||
        CU_add_test(my_test_suite, "add same item twice", test_add_same_item_twice) == NULL ||
        CU_add_test(my_test_suite, "add to cart insufficient stock", 
            test_add_to_cart_insufficient_stock) == NULL ||
        CU_add_test(my_test_suite, "remove from cart not in cart", 
            test_remove_from_cart_not_in_cart) == NULL ||
        CU_add_test(my_test_suite, "remove cart releases reservation", 
            test_remove_cart_releases_reservation) == NULL ||
        CU_add_test(my_test_suite, "checkout single location", test_checkout_single_location) == NULL ||
        CU_add_test(my_test_suite, "checkout empties location", test_checkout_empties_location) == NULL ||
        CU_add_test(my_test_suite, "checkout multiple items", test_checkout_multiple_items) == NULL
    )
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

    CU_cleanup_registry();

    return CU_get_error();
}