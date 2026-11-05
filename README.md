## Database emulator

### Tests and coverage
Tests are implemented for the db_backend file.

- To make and run tests and review coverage:
	`make coverage`
- To make and run a memory test using an input file (webstoretest.txt) to run commands:
	`make memrun`
- To make the executable for manual testing: 
	`make frontend`

As of now the coverage of the tests is as following: 
``` Bash
File                                    Branches    Taken  Cover   Missing
------------------------------------------------------------------------------
db_backend.c                                  80       64    80%   93,99,134,175,179,200,213,247,253,262,281,377,439,454,515
------------------------------------------------------------------------------
TOTAL                                         80       64    80%
----------------------------------------------------------------------------
```
### Summary
This consists of two files that are dependent on the IOOPM hashtable and linked list implementation.

It's purpose is to simulate a webstore backend where you can store, edit and retrieve items stored. You can also create and manage carts like a user would. 

The db_backend files contain backend functions that manipulates hashtables where items are stored, while the frontend.c file is used as an interactive way to interact with the backend. 

- Hash tables: Store items by name (`ht_n_i`) and map storage locations to item names (`ht_sl_n`).
- Linked lists: Store an item's stock entries and a cart's contents.
- `item_t`: Represents a merchandise item, including name, description, price and stock information.
- `stock_entry_t`: Represents a storage location and its quantity of a particular item.
- `cart_t`: Represents a shopping cart with an ID, total cost and list of cart entries.
- `cart_elem_t`: Stores an item name and the quantity requested in a cart.

The system uses two hash tables: one maps item names to item structs, while the other maps shelf locations to item names. Each item struct contains its information and a linked list of stock entries, tracking quantities at different locations. Shopping carts are stored separately, each containing a linked list of items and their requested quantities.

You can find documentation in the db_backend.h file where each function is described. 

#### Ownership
The database takes copies of all strings and structures inserted. You need to manually free strings after they are inserted. After that the database itself is responsible for freeing its copies. 

