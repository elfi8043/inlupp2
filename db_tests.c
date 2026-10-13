#include <CUnit/Basic.h>
#include "db_backend.h"
#include "hash_table_iterator.h"
#include "hash_table.h"
#include "linked_list.h"
#include "list_iterator.h"
#include <stdlib.h>

int init_suite(void) {
  // Change this function if you want to do something *before* you
  // run a test suite
  return 0;
}

int clean_suite(void) {
  // Change this function if you want to do something *after* you
  // run a test suite
  return 0;
}

// These are example test functions. You should replace them with
// functions of your own.
void test_add_merch(void) {
  ioopm_hash_table_t *ht_n_i = ioopm_hash_table_create(ioopm_hash_string, ioopm_string_comp);
  char *name = "Jonkanon";
  char *desc = "Kanon som skjuter jon";
  int price = 100;
  item_t *item = db_create_item(name, desc, price);

  CU_ASSERT_EQUAL(db_add_merch(ht_n_i, item), DB_OK);
  elem_t result = int_elem(-1);
  CU_ASSERT_TRUE(ioopm_hash_table_has_key(ht_n_i, string_elem(name)));
  CU_ASSERT_EQUAL(db_add_merch(ht_n_i, item), DB_MERCH_EXISTS);
  db_ht_destroy(ht_n_i);
}

void test_list_merch(void) {
  ioopm_hash_table_t *ht_n_i = ioopm_hash_table_create(ioopm_hash_string, ioopm_string_comp);
  
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
      "Hu va av slitstarkt lader",
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
  

}

int main() {
  // First we try to set up CUnit, and exit if we fail
  if (CU_initialize_registry() != CUE_SUCCESS)
    return CU_get_error();

  // We then create an empty test suite and specify the name and
  // the init and cleanup functions
  CU_pSuite my_test_suite = CU_add_suite("My awesome test suite", init_suite, clean_suite);
  if (my_test_suite == NULL) {
      // If the test suite could not be added, tear down CUnit and exit
      CU_cleanup_registry();
      return CU_get_error();
  }

  // This is where we add the test functions to our test suite.
  // For each call to CU_add_test we specify the test suite, the
  // name or description of the test, and the function that runs
  // the test in question. If you want to add another test, just
  // copy a line below and change the information
  if (
    (CU_add_test(my_test_suite, "Add merch", test_add_merch) == NULL) ||
    (CU_add_test(my_test_suite, "Basic arithmetics", test2) == NULL) ||
    0
  )
    {
      // If adding any of the tests fails, we tear down CUnit and exit
      CU_cleanup_registry();
      return CU_get_error();
    }

  // Set the running mode. Use CU_BRM_VERBOSE for maximum output.
  // Use CU_BRM_NORMAL to only print errors and a summary
  CU_basic_set_mode(CU_BRM_VERBOSE);

  // This is where the tests are actually run!
  CU_basic_run_tests();

  // Tear down CUnit before exiting
  CU_cleanup_registry();
  return CU_get_error();
} 