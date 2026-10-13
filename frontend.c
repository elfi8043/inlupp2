// Goes functions that prints stuff
// and error handling for each function
// there should be atleast one function per action

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "../../labbar/lab3/utils2.h"
#include "webstore.h"

#define LIST_PAGE_SIZE 20
#define VALID_ACTIONS "ALDESPCR+-=OUQ"

static bool accept_any(char *str)
{
  (void) str;
  return true;
}

static bool is_positive_int(char *str)
{
  return is_number(str) && atoi(str) >= 1;
}

static bool is_non_negative_int(char *str)
{
  return is_number(str) && atoi(str) >= 0;
}

static char *ask_question_allow_empty(char *question)
{
  return ask_question(question, accept_any, convert_to_string).string_value;
}

/// Ställer en ja/nej-fråga. Sant om svaret börjar på 'Y' eller 'y'.
static bool ask_confirmation(char *question)
{
  char *answer = ask_question_allow_empty(question);
  bool yes = toupper((unsigned char) answer[0]) == 'Y';
  free(answer);
  return yes;
}

static int ask_positive_int(char *question)
{
  return ask_question(question, is_positive_int, convert_to_int).int_value;
}

static int ask_non_negative_int(char *question)
{
  return ask_question(question, is_non_negative_int, convert_to_int).int_value;
}

/// check_func: en lagerplats är en versal A-Z följd av exakt två siffror, t.ex. "B07".
static bool is_valid_location(char *s)
{
  return strlen(s) == 3
      && s[0] >= 'A' && s[0] <= 'Z'
      && isdigit((unsigned char) s[1])
      && isdigit((unsigned char) s[2]);
}

static char *ask_location(char *question)
{
  return ask_question(question, is_valid_location, convert_to_string).string_value;
}

static void print_status(ws_status_t status)
{
  switch (status)
  {
  case WS_OK:                 break;
  case WS_MERCH_EXISTS:       puts("Fel: det finns redan en vara med det namnet."); break;
  case WS_NO_SUCH_MERCH:      puts("Fel: varan finns inte."); break;
  case WS_NO_SUCH_CART:       puts("Fel: kundvagnen finns inte."); break;
  case WS_LOCATION_TAKEN:     puts("Fel: lagerplatsen innehåller redan en annan vara."); break;
  case WS_INSUFFICIENT_STOCK: puts("Fel: det finns inte tillräckligt många i lager."); break;
  case WS_NOT_IN_CART:        puts("Fel: varan finns inte i kundvagnen."); break;
  }
}

/// Skriver ut success_msg om allt gick bra, annars ett felmeddelande.
static void print_result(ws_status_t status, const char *success_msg)
{
  if (status == WS_OK) puts(success_msg);
  else print_status(status);
}

static void print_menu(void)
{
  puts("\n[A] Lägg till vara       [L] Lista varor          [D] Ta bort vara\n"
       "[E] Redigera vara        [S] Visa lager           [P] Fyll på lager\n"
       "[C] Skapa kundvagn       [R] Ta bort kundvagn     [+] Lägg i kundvagn\n"
       "[-] Ta ur kundvagn       [=] Beräkna kostnad      [O] Gå till kassan\n"
       "[U] Ångra                [Q] Avsluta");
}

static void ui_add_merch(webstore_t *ws)
{
  char *name = ask_question_string("Varans namn:");
  char *desc = ask_question_string("Beskrivning:");
  int price = ask_non_negative_int("Pris (SEK):");
  item_t *item = db_create_item(name, desc, price);

  print_result(ws_add_merch(ws, item), "Varan lades till.");

  free(name);
  free(desc);
}

static void ui_list_merch(webstore_t *ws)
{
  size_t count = 0;
  ws_merch_info_t *merch = ws_list_merch(ws, &count);

  if (count == 0) puts("Det finns inga varor.");

  for (size_t i = 0; i < count; i++)
  {
    printf("%zu. %s - %s (%d SEK)\n", i + 1, merch[i].name, merch[i].desc, merch[i].price);

    bool page_full = (i + 1) % LIST_PAGE_SIZE == 0;
    bool more_left = i + 1 < count;
    if (page_full && more_left)
    {
      char *answer = ask_question_allow_empty("Visa fler? (N för att återgå till menyn)");
      bool stop = toupper((unsigned char) answer[0]) == 'N';
      free(answer);
      if (stop) break;
    }
  }

  free(merch);
}

static void ui_remove_merch(webstore_t *ws)
{
  char *name = ask_question_string("Namn på varan som ska tas bort:");

  if (ask_confirmation("Är du säker? (y/n)"))
  {
    print_result(ws_remove_merch(ws, name), "Varan togs bort.");
  }
  else
  {
    puts("Ingenting togs bort.");
  }

  free(name);
}

static void ui_edit_merch(webstore_t *ws)
{
  char *old_name = ask_question_string("Namn på varan som ska ändras:");
  char *new_name = ask_question_string("Nytt namn:");
  char *new_desc = ask_question_string("Ny beskrivning:");
  int new_price = ask_non_negative_int("Nytt pris (SEK):");

  if (ask_confirmation("Spara ändringarna? (y/n)"))
  {
    print_result(ws_edit_merch(ws, old_name, new_name, new_desc, new_price),
                 "Varan uppdaterades.");
  }
  else
  {
    puts("Ingenting ändrades.");
  }

  free(old_name);
  free(new_name);
  free(new_desc);
}

static void ui_show_stock(webstore_t *ws)
{
  char *name = ask_question_string("Varans namn:");

  size_t count = 0;
  ws_status_t status = WS_OK;
  ws_stock_entry_t *stock = ws_show_stock(ws, name, &count, &status);

  if (status != WS_OK)
  {
    print_status(status);
  }
  else if (count == 0)
  {
    printf("%s finns inte i lager.\n", name);
  }
  else
  {
    for (size_t i = 0; i < count; i++)
    {
      printf("%s: %d st\n", stock[i].location, stock[i].quantity);
    }
  }

  free(stock);
  free(name);
}

static void ui_replenish(webstore_t *ws)
{
  char *location = ask_location("Lagerplats:");
  char *name = ask_question_string("Varans namn:");
  int amount = ask_positive_int("Antal att lägga till:");

  print_result(ws_replenish(ws, location, name, amount), "Lagret fylldes på.");

  free(location);
  free(name);
}

static void ui_create_cart(webstore_t *ws)
{
  int id = ws_create_cart(ws);
  printf("Kundvagn %d skapades.\n", id);
}

static void ui_remove_cart(webstore_t *ws)
{
  int id = ask_question_int("Kundvagnens id:");

  if (ask_confirmation("Är du säker? (y/n)"))
  {
    print_result(ws_remove_cart(ws, id), "Kundvagnen togs bort.");
  }
  else
  {
    puts("Ingenting togs bort.");
  }
}

static void ui_add_to_cart(webstore_t *ws)
{
  int id = ask_question_int("Kundvagnens id:");
  char *name = ask_question_string("Varans namn:");
  int amount = ask_positive_int("Antal:");

  print_result(ws_add_to_cart(ws, id, name, amount), "Varan lades i kundvagnen.");

  free(name);
}

static void ui_remove_from_cart(webstore_t *ws)
{
  int id = ask_question_int("Kundvagnens id:");
  char *name = ask_question_string("Varans namn:");
  int amount = ask_non_negative_int("Antal:");

  print_result(ws_remove_from_cart(ws, id, name, amount), "Varan togs ur kundvagnen.");

  free(name);
}

static void ui_cart_cost(webstore_t *ws)
{
  int id = ask_question_int("Kundvagnens id:");

  int cost = 0;
  ws_status_t status = ws_cart_cost(ws, id, &cost);

  if (status == WS_OK) printf("Kundvagn %d kostar %d SEK.\n", id, cost);
  else print_status(status);
}

static void ui_checkout(webstore_t *ws)
{
  int id = ask_question_int("Kundvagnens id:");
  print_result(ws_checkout(ws, id), "Köpet genomfördes.");
}

static char ask_action(void)
{
  while (true)
  {
    char *answer = ask_question_string("Välj kommando:");
    char action = (char) toupper((unsigned char) answer[0]);
    bool valid = strlen(answer) == 1 && strchr(VALID_ACTIONS, action) != NULL;
    free(answer);

    if (valid) return action;
    puts("Ogiltigt kommando, försök igen.");
  }
}

static void event_loop(webstore_t *ws)
{
  bool running = true;

  while (running)
  {
    print_menu();

    switch (ask_action())
    {
    case 'A': ui_add_merch(ws);        break;
    case 'L': ui_list_merch(ws);       break;
    case 'D': ui_remove_merch(ws);     break;
    case 'E': ui_edit_merch(ws);       break;
    case 'S': ui_show_stock(ws);       break;
    case 'P': ui_replenish(ws);        break;
    case 'C': ui_create_cart(ws);      break;
    case 'R': ui_remove_cart(ws);      break;
    case '+': ui_add_to_cart(ws);      break;
    case '-': ui_remove_from_cart(ws); break;
    case '=': ui_cart_cost(ws);        break;
    case 'O': ui_checkout(ws);         break;
    case 'U': puts("Ångra är inte implementerat."); break;
    case 'Q': running = !ask_confirmation("Vill du avsluta? (y/n)"); break;
    }
  }
}

int main(void)
{
  webstore_t *ws = ws_create();
  event_loop(ws);
  ws_destroy(ws);
  return 0;
}