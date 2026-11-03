#include "db_backend.h"
#include <stdbool.h>
#include "utils.h"

static void print_menu(void)
{
    puts("\n========== DB MENU ==========");
    puts("A - Add merchandise");
    puts("L - List all merchandise");
    puts("D - Remove merchandise");
    puts("E - Edit merchandise");
    puts("S - Show merchandise stock");
    puts("P - Replenish merchandise");
    puts("--------------------------------");
    puts("C - Create shopping cart");
    puts("R - Remove shopping cart");
    puts("+ - Add item to cart");
    puts("- - Remove item from cart");
    puts("= - Show cart cost");
    puts("O - Checkout cart");
    puts("--------------------------------");
    puts("U - Undo last action");
    puts("Q - Quit");
    puts("================================");
}

static char ask_action()
{
  return ask_question_char("Enter an action: ");
}

static void ui_add_merch(ioopm_hash_table_t *ht_n_i)
{
    puts("\nAdd Item: ");
    char *name = ask_question_string("Name: ");
    char *desc = ask_question_string("\nDescription: ");
    int price = ask_question_int("\nPrice: ");
    item_t *item = db_create_item(name, desc, price);
    db_status_t status = db_add_merch(ht_n_i, item);
    if (status == DB_OK)
    {
      puts("\nItem added.");
      free(name);
      free(desc);
      return;
    } else
    {
      puts("\nError: Merch exists");
      free(name);
      free(desc);
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
    free(name);
    return;
  } else
  {
    puts("\nNo such merch exist");
    free(name);
    return;
  }
}

static void ui_edit_merch(ioopm_hash_table_t *ht_n_i, ioopm_hash_table_t *ht_sl_n)
{
  puts("Edit merch: ");
  char *old_name = ask_question_string("Enter name of item to edit:");
  char *name = ask_question_string("\nEnter new name: ");
  char *desc = ask_question_string("\nEnter new description: ");
  int price = ask_question_int("\nEnter new price: ");
  char *confirm = ask_question_string("\nConfirm (y/n): ");

  if (confirm[0] == 'y' || confirm[0] == 'Y')
  {
    db_status_t status = db_edit_merch(ht_n_i, ht_sl_n, old_name, name, desc, price);
    if (status == DB_OK)
    {
      puts("Item edited.");
      free(old_name);
      free(name);
      free(desc);
      free(confirm);
      return;
    } else
    {
      puts("Error: No such merch");
      free(old_name);
      free(name);
      free(desc);
      free(confirm);
      return;
    }
  } else
  {
    puts("Cancelled");
    free(old_name);
    free(name);
    free(desc);
    free(confirm);
    return;
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
      char *confirm = ask_question_string("Enter N or n to break, else continue");
      if (confirm[0] == 'n' || confirm[0] == 'N') {
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
    free(name);
    free(sl);
    return; 
  } else if (status == DB_LOCATION_TAKEN)
  {
    puts("Error: Location occupied by another item.");
    free(name);
    free(sl);
    return;
  } else if (status == DB_NO_SUCH_MERCH)
  {
    puts("Error: No such merch.");
    free(name);
    free(sl);
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

static cart_t *get_cart(ioopm_list_t *carts, int id)
{
  ioopm_list_iterator_t *iter = ioopm_list_iterator_create(carts);
  while (!ioopm_list_iterator_at_end(iter))
  {
    cart_t *curr = ioopm_list_iterator_current(iter).p;
    if (curr->id == id)
    {
      ioopm_list_iterator_destroy(iter);
      return curr;
    }
    ioopm_list_iterator_advance(iter);
  }
  ioopm_list_iterator_destroy(iter);
  return NULL;
  
}

static void remove_cart(ioopm_list_t *carts, int id, int *no_carts)
{
  ioopm_list_iterator_t *iter = ioopm_list_iterator_create(carts);
  while (!ioopm_list_iterator_at_end(iter))
  {
    cart_t *curr = ioopm_list_iterator_current(iter).p;
    if (curr->id == id)
    {
      cart_t *cart = ioopm_list_iterator_remove(iter).p;
      *no_carts -= 1;
    }
    ioopm_list_iterator_advance(iter);
  }
  ioopm_list_iterator_destroy(iter);
  return;
}

static void ui_remove_cart(ioopm_list_t *carts, int *no_carts)
{
  puts("Remove cart: ");
  int id = ask_question_int("Enter id of cart to remove: ");
  char *confirm = ask_question_string("Confirm (y/n): ");
  if (confirm[0] == 'y' || confirm[0] == 'Y')
  {
    cart_t *cart = get_cart(carts, id);
    if (cart)
    {
      db_status_t status = db_remove_cart(cart);
      if (status == DB_OK)
      {
        // remove cart from list of carts
        remove_cart(carts, id, no_carts);
        puts("Cart removed.");
        return;
      } 
    } else 
    {
    puts("Error: No such cart");
    return;
    }
  }
  else 
  {
    puts("Cancelled");
  }
}

static void ui_add_to_cart(ioopm_hash_table_t *ht_n_i, ioopm_list_t *carts)
{
  puts("Add to cart: ");
  int id = ask_question_int("Enter id of cart to add to: ");
  cart_t *cart = get_cart(carts, id);
  if (cart == NULL)
  {
    puts("Error: No such cart");
    return;
  }
  char *name = ask_question_string("\nEnter name of item to add: ");
  int quant = ask_question_int("\nEnter quantity to add: ");
  item_t *item = NULL;

  db_status_t status = db_get_item(ht_n_i, name, &item);
  if (status == DB_OK)
  {
    db_status_t status_cart = db_add_to_cart(cart, item, quant);
    if (status_cart == DB_OK)
    {
      puts("Item Added.");
      free(name);
      return;
    } else if (status_cart == DB_NO_SUCH_CART)
    {
      puts("Error: No such cart.");
      free(name);
      return;
    }

  } else if (status == DB_NO_SUCH_MERCH)
  {
    puts("Error: No such merch.");
    free(name);
    return;
  } else if (status == DB_INSUFFICIENT_STOCK)
  {
    puts("Error: Insufficient stock");
    free(name);
    return;
  }
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
         printf("Location: %s \n Quantity: \n  %d", curr->sl, curr->quant);

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
      char *confirm = ask_question_string("No such merch, enter Y to try again");
      if (confirm[0] == 'y' || confirm[0] == 'Y') {
        continue;
      }
      else {
        free(name);
        break;
      }
    }
    else {
      free(name);
      break;
    }
  }
}

static void print_cart(cart_t *cart) {
  ioopm_list_iterator_t *iter =
        ioopm_list_iterator_create(cart->l);

    while (!ioopm_list_iterator_at_end(iter))
    {
        cart_elem_t *curr = ioopm_list_iterator_current(iter).p;
        printf("Name: %s \n Quantaty: %d", curr->name, curr->quant);
        ioopm_list_iterator_advance(iter);
    }
    ioopm_list_iterator_destroy(iter);
}

static void ui_remove_from_cart(ioopm_hash_table_t *ht_n_i, ioopm_list_t *carts) {
  int id;
  cart_t *cart;
  while (true) {
    id = ask_question_int("Enter id of cart to remove from: ");
    cart = get_cart(carts, id);
    if (cart == NULL) {
      char *confirm = ask_question_string("No such cart id, enter \"n\" to exit");
      if (confirm[0] == 'n' || confirm[0] == 'N')  {
        return;
      }
      else {
        continue;
      }
    }
    else {
      break;
    }
  }
  print_cart(cart);
  char *name = ask_question_string("What merch do you want to remove?");
  int quant = ask_question_int("How many of the merch du you want to remove?");
  db_status_t status = db_remove_from_cart(ht_n_i, cart, name, quant);
  if (status == DB_NO_SUCH_MERCH) {
    puts("No such merch");
    free(name);
  }
  else if (status == DB_NOT_IN_CART) {
    puts("No such merch in cart");
    free(name);
  }
}

static void ui_cart_cost(ioopm_list_t *carts) {
  while (true) {
    int id = ask_question_int("Enter id of cart to evaluate: ");
    cart_t *cart = get_cart(carts, id);
    if (cart == NULL) {
      char *confirm = ask_question_string("No such cart id, enter Y to try again");
      if (confirm[0] == 'y' || confirm[0] == 'Y')  {
        free(confirm);
        continue;
      }
      else {
        free(confirm);
        break;
      }
    }
    else {
      printf("Total cart cost: %d", cart->total_cost);
      break;
    }
  }
}

static void ui_checkout(ioopm_hash_table_t *ht_n_i, 
  ioopm_hash_table_t *ht_sl_n, ioopm_list_t *carts) {
  int id;
  cart_t *cart;
  while (true) {
    id = ask_question_int("Enter id of cart to checkout: ");
    cart = get_cart(carts, id);
    if (cart == NULL) {
      char *confirm = ask_question_string("No such cart id, enter Y to try again");
      if (confirm[0] == 'y' || confirm[0] == 'Y') {
        free(confirm);
        continue;
      }
      else {
        free(confirm);
        return;
      }
    }
    else {
      break;
    }
  }
  int cost = db_checkout(cart, ht_n_i, ht_sl_n);
  printf("Cost of cart: %d", cost);
}

static bool ask_confirmation(char *question)
{
  char cont;
  while (true)
  {
    cont = ask_question_char(question);
    if (cont == 'y' || cont == 'Y')
    {
      return true;
    } else if (cont == 'n' || cont == 'N')
    {
      return false;
    } else 
    {
      puts("Invalid input type y/n");
    }
  }
}

static void event_loop(ioopm_hash_table_t *ht_n_i, ioopm_hash_table_t *ht_sl_n, ioopm_list_t *carts, int *no_carts)  
{
  bool running = true;

  while (running)
  {
    print_menu();

    switch (ask_action())
    {
    case 'A': ui_add_merch(ht_n_i);        break;
    case 'L': ui_list_merch(ht_n_i);       break;
    case 'D': ui_remove_merch(ht_n_i, ht_sl_n);     break;
    case 'E': ui_edit_merch(ht_n_i, ht_sl_n);       break;
    case 'S': ui_show_stock(ht_n_i);       break;
    case 'P': ui_replenish(ht_n_i, ht_sl_n);        break;
    case 'C': ui_create_cart(carts, no_carts);      break;
    case 'R': ui_remove_cart(carts, no_carts);      break;
    case '+': ui_add_to_cart(ht_n_i, carts);      break;
    case '-': ui_remove_from_cart(ht_n_i, carts); break;
    case '=': ui_cart_cost(carts);        break;
    case 'O': ui_checkout(ht_n_i, ht_sl_n, carts);         break;
    case 'U': puts("Ångra är inte implementerat."); break;
    case 'Q': running = !ask_confirmation("Vill du avsluta? (y/n)"); break;
    }
  }
}

int main(int argc, char *argv[])
{
    // initialize databases
    ioopm_hash_table_t *ht_n_i = ioopm_hash_table_create(ioopm_hash_string, ioopm_string_comp);
    ioopm_hash_table_t *ht_sl_n = ioopm_hash_table_create(ioopm_hash_string, ioopm_string_comp);

    // initialize cart list
    ioopm_list_t *carts = ioopm_list_create();
    int no_carts = 0;

    event_loop(ht_n_i, ht_sl_n, carts, &no_carts);
    db_ht_destroy(ht_n_i, ht_sl_n);

    // TODO: Cleanup list of carts

    return 0;
}