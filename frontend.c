#include "db_backend.h"
#include <stdbool.h>
#include "utils.h"

int main(int argc, char *argv[])
{
    // initialize databases
    ioopm_hash_table_t *ht_n_i = ioopm_hash_table_create(ioopm_hash_string, ioopm_string_comp);
    ioopm_hash_table_t *ht_sl_n = ioopm_hash_table_create(ioopm_hash_string, ioopm_string_comp);

    // initialize cart list
    ioopm_list_t *carts = ioopm_list_create();
    int no_carts = 0;

    event_loop(ht_n_i, ht_sl_n, carts, &no_carts);
}

void ui_add_merch(ioopm_hash_table_t *ht_n_i)
{
    puts("\nAdd Item: ");
    char *name = ask_question_string("Name: ");
    char *desc = ask_question_string("\nDescription: ");
    int *price = ask_question_int("\nPrice: ");
    item_t *item = db_create_item(name, desc, price);
    db_status_t status = db_add_merch(ht_n_i, item);
    if (status == DB_OK)
    {
      puts("\nItem added.");
      return;
    } else
    {
      puts("\nError: Merch exists");
      return;
    }
}

static void ui_remove_merch(ioopm_hash_table_t *ht_n_i, ioopm_hash_table_t *ht_sl_n)
{
  puts("Remove Item:");
  char *name = ask_question_string("Enter name of item to remove: ");
  db_status_t status = db_remove_merch(ht_n_i, ht_sl_n, name);
  if (status == DB_OK)
  {
    puts("\nItem removed.");
    return;
  } else
  {
    puts("\nNo such merch exist");
    return;
  }
}

static void ui_edit_merch(ioopm_hash_table_t *ht_n_i, ioopm_hash_table_t *ht_sl_n)
{
  puts("Edit merch: ");
  char *old_name = ask_question_string("Enter name of item to edit:");
  char *name = ask_question_string("\nEnter new name: ");
  char *desc = ask_question_string("\nEnter new description: ");
  int *price = ask_question_int("\nEnter new price: ");
  char *confirm = ask_question_string("\nConfirm (y/n): ");

  if (confirm[0] == "y" || confirm[0] == "Y")
  {
    db_status_t status = db_edit_merch(ht_n_i, ht_sl_n, old_name, name, desc, price);
    if (status == DB_OK)
    {
      puts("Item edited.");
      return;
    } else
    {
      puts("Error: No such merch");
      return;
    }
  } else
  {
    puts("Cancelled");
  }
}

static void ui_list_merch(ioopm_hash_table_t *ht_n_i) {
  ioopm_hash_table_iterator_t *iter = ioopm_hash_table_iterator_create(ht_n_i);
  int count = 0;

  while (!ioopm_hash_table_iterator_at_end(iter)) {
    item_t *item = ioopm_hash_table_iterator_current_value(iter).p;
    int total_left = item->total_stock - item->total_reserved;
    printf("name: %s \n price: \n  %d,desc:  %s\n  items left:  %d \n", 
    item->name, item->price, item->desc, total_left);

    ioopm_hash_table_iterator_advance(iter);
    count++;
    if (count % 20 == 0) {
      char *ans = ask_question_string("Enter N or n to break, else continue");
      if (strcmp(ans, 'N') == 0 || strcmp(ans, 'n') == 0) {
        break;
      }
      else {
        continue;
      }
    }
  }
  ioopm_hash_table_iterator_destroy(iter);
}

static void ui_replenish(ioopm_hash_table_t *ht_n_i, ioopm_hash_table_t *ht_sl_n)
{
  puts("Replenish item: ");
  char *name = ask_question_string("Enter name of item to replenish: ");
  int quant = ask_question_int("\nEnter replenish item amount: ");
  char *sl = ask_question_shelf("\nEnter storage location to replenish to: ");
  db_status_t status = db_replenish(ht_n_i, ht_sl_n, name, quant, sl);
  if (status == DB_OK)
  {
    puts("Item replenished");
    return; 
  } else if (status == DB_LOCATION_TAKEN)
  {
    puts("Error: Location occupied by another item.");
    return;
  } else if (status == DB_NO_SUCH_MERCH)
  {
    puts("Error: No such merch.");
    return;
  } 
}

static void ui_create_cart(ioopm_list_t *carts, int *no_carts)
{
  puts("Create cart: ");
  cart_t *cart = db_create_cart(no_carts);
  ioopm_list_append(carts, ptr_elem(cart));
  printf("Your cart id is %d", cart->id);
}

cart_t *get_cart(ioopm_list_t *carts, int id)
{
  ioopm_list_iterator_t *iter = ioopm_hash_table_iterator_create(carts);
  while (!ioopm_list_iterator_at_end(iter))
  {
    cart_t *curr = ioopm_list_iterator_current(iter).p;
    if (curr->id = id)
    {
      return curr;
    }
  }
  
}

static void ui_remove_cart(ioopm_list_t *carts, int *no_carts)
{
  puts("Remove cart: ");
  int id = ask_question_int("Enter id of cart to remove: ");
  cart_t *cart = get_cart(carts, id);
  db_status_t status = db_remove_cart(cart);
}

static db_status_t ui_show_stock_print(ioopm_hash_table_t *ht_n_i, char *name) {
  elem_t elem;
  bool exists = ioopm_hash_table_lookup(ht_n_i, string_elem(name), &elem);
  if (!exists) {
    return DB_NO_SUCH_MERCH;
  }
  item_t *item = elem.p;
  ioopm_list_iterator_t *iter =
        ioopm_list_iterator_create(item->l);

    while (!ioopm_list_iterator_at_end(iter))
    {
        stock_entry_t *curr = ioopm_list_iterator_current(iter).p;
         printf("Location: %s \n Quantaty: \n  %d", curr->sl, curr->quant);

        ioopm_list_iterator_advance(iter);
    }

  ioopm_list_iterator_destroy(iter);
  return DB_OK;
}

static void ui_show_stock(ioopm_hash_table_t *ht_n_i) {
  while (true) {
    char *name = ask_question_string("Which items stock do you want to see?");
    db_status_t status = ui_show_stock_print(ht_n_i, name);
    if (status == DB_NO_SUCH_MERCH) {
      char *ans = ask_question_string("No such merch, enter Y to try again");
      if (strcmp(ans, 'y') == 0 || strcmp(ans, 'Y') == 0) {
        continue;
      }
      else {
        break;
      }
    }
    else {
      break;
    }
  }
}

static void event_loop(ioopm_hash_table_t *ht_n_i, ioopm_hash_table_t *ht_sl_n, ioopm_list_t *carts, int *no_carts)  
{
  bool running = true;

  while (running)
    print_menu();

    switch (ask_action())
    {
    case 'A': ui_add_merch();        break;
    case 'L': ui_list_merch();       break;
    case 'D': ui_remove_merch();     break;
    case 'E': ui_edit_merch();       break;
    case 'S': ui_show_stock();       break;
    case 'P': ui_replenish();        break;
    case 'C': ui_create_cart();      break;
    case 'R': ui_remove_cart();      break;
    case '+': ui_add_to_cart();      break;
    case '-': ui_remove_from_cart(); break;
    case '=': ui_cart_cost();        break;
    case 'O': ui_checkout();         break;
    case 'U': puts("Ångra är inte implementerat."); break;
    case 'Q': running = !ask_confirmation("Vill du avsluta? (y/n)"); break;
    }
  }
}

