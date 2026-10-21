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
        CU_add_test(my_test_suite, "remove merch", test_remove_merch) == NULL
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