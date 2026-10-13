#include "hash_table.h"
#include "hash_table_iterator.h"
#include "common.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

typedef struct entry entry_t;

#define No_buckets 17

struct entry
{
    elem_t key;    // holds the key
    elem_t value;  // holds the value
    entry_t *next; // points to the next entry (possibly NULL)
};

struct hash_table
{
    entry_t buckets[No_buckets];  // holds buckets of amount No_buckets
    size_t size;                  // size of hash table
    ioopm_eq_function *eq_fn;     // check equality function
    ioopm_hash_function *hash_fn; // gives hash value of given key
};

ioopm_hash_table_t *ioopm_hash_table_create(ioopm_hash_function *hash_fn, ioopm_eq_function *key_eq_fn)
{
    // allocate memory for hash table
    ioopm_hash_table_t *ht = calloc(1, sizeof(ioopm_hash_table_t));
    // assign functions to hash table
    ht->hash_fn = hash_fn;
    ht->eq_fn = key_eq_fn;
    return ht;
}

// Recursive function, iterative process
// static void destroy_linked_list(entry_t *curr)
//{
//    // free curr if it is the last entry
//    if (curr->next == NULL)
//    {
//        free(curr);
//    }
//    // free current and make recursive call with next entry
//    else
//    {
//        entry_t *next = curr->next;
//        free(curr);
//        destroy_linked_list(next);
//    }
//}

// Recursive function recursive process
// static void destroy_linked_list(entry_t *curr)
//{
//    // free curr if it is the last entry
//     if (curr->next == NULL)
//     {
//         free(curr);
//     }
//    // make recursive call with next entry and then free current
//     else
//     {
//         // free current
//         destroy_linked_list(curr->next);
//         free(curr);
//     }
// }

// iterative function iterative process
/// @brief destroys a linked list with nodes entry_t
/// @param curr list to destroy
static void destroy_linked_list(entry_t *curr)
{
    // free current entry while curr is not NULL
    while (curr != NULL)
    {
        entry_t *new_curr = curr->next;
        free(curr);
        curr = new_curr;
    }
}

void ioopm_hash_table_destroy(ioopm_hash_table_t *ht)
{
    // goes through all buckets of the hash table
    for (int i = 0; i < No_buckets; i++)
    {
        // the current list
        entry_t *curr_list = ht->buckets[i].next;
        // continues if current list is empty
        if (curr_list == NULL)
        {
            continue;
        }
        // else destroys current list
        else
        {
            destroy_linked_list(curr_list);
        }
    }
    free(ht);
}

/// @brief creates an entry
/// @param key key of the entry
/// @param value value of the entry
/// @param next the next entry to point to
/// @return an entry of type entry_t
static entry_t *entry_create(elem_t key, elem_t value, entry_t *next)
{
    // allocate memory for entry and assigns key, value, and pointer to the next entry
    entry_t *entry = calloc(1, sizeof(entry_t));
    entry->key = key;
    entry->value = value;
    entry->next = next;
    return entry;
}

/// @brief finds the previous entry to the current key key in hash table ht
/// @param ht hash table operated upon
/// @param key key of current entry
/// @return the previous entry, or the last entry if the key does not exist
static entry_t *find_previous_entry(ioopm_hash_table_t *ht, elem_t key)
{
    // bucket get hash value of key
    size_t bucket = ht->hash_fn(key) % No_buckets;
    // list in buckets corresponding to bucket
    entry_t *list = &ht->buckets[bucket];
    // runs while not at the end of the list
    while (list->next != NULL)
    {
        // breaks the loop if next entrys key is equal to key
        if (ht->eq_fn(list->next->key, key))
        {
            break;
        }
        list = list->next;
    }
    return list;
}

void ioopm_hash_table_insert(ioopm_hash_table_t *ht, elem_t key, elem_t value)
{
    // find previous entry, or the last entry if the key does not exist
    entry_t *previous = find_previous_entry(ht, key);

    // if the key exists, update the value
    if (previous->next != NULL)
    {
        previous->next->value = value;
    }
    // else ,create a new entry
    else
    {
        previous->next = entry_create(key, value, NULL);
        ht->size += 1;
    }
}

bool ioopm_hash_table_lookup(ioopm_hash_table_t *ht, elem_t key, elem_t *result)
{
    // find previous entry, or the last entry if the key does not exist
    entry_t *previous = find_previous_entry(ht, key);

    // returns false if current entry is NULL (when previous is the last entry)
    if (previous->next == NULL)
    {
        return false;
    }
    // returns true and gives result the current value, if current entrys key is the one we are looking for
    if (ht->eq_fn(previous->next->key, key))
    {
        *result = previous->next->value;
        return true;
    }
    // else returns false
    else
    {
        return false;
    }
}

bool ioopm_hash_table_remove(ioopm_hash_table_t *ht, elem_t key, elem_t *result)
{
    // find previous entry, or the last entry if the key does not exist
    entry_t *previous = find_previous_entry(ht, key);

    // returns false if current entry is NULL (when previous is the last entry)
    if (previous->next == NULL)
    {
        return false;
    }
    // check if current entrys key is the one we are looking for
    if (ht->eq_fn(previous->next->key, key))
    {
        // gives result the current value to be removed
        *result = previous->next->value;
        // removes current entry if at end of list
        if (previous->next->next == NULL)
        {
            entry_t *temp = previous->next;
            previous->next = NULL;
            free(temp);
        }
        // else update current entry and then remove the previous current entry
        else
        {
            entry_t *temp = previous->next;
            previous->next = previous->next->next;
            free(temp);
        }
        ht->size -= 1;
        return true;
    }
    else
    {
        return false;
    }
}

bool ioopm_hash_table_has_key(ioopm_hash_table_t *ht, elem_t key)
{
    // returns true if hash table contains key
    elem_t result = int_elem(-1);
    return ioopm_hash_table_lookup(ht, key, &result);
}

bool ioopm_hash_table_is_empty(ioopm_hash_table_t *ht)
{
    return (ht->size == 0);
}

size_t ioopm_hash_table_size(ioopm_hash_table_t *ht)
{
    return ht->size;
}

// hash function taken from chatgpt
size_t ioopm_hash_int(elem_t key)
{
    uint32_t x = (uint32_t)key.i;
    x ^= x >> 16;
    x = 0x7feb352d;
    x ^= x >> 15;
    x = 0x846ca68b;
    x ^= x >> 16;

    return (size_t)x;
}

// TODO: create a function of type ioopm_hash_function that creates a hash value for type unsigned int

// TODO: create a function of type ioopm_hash_function that creates a hash value for type bool

// TODO: create a function of type ioopm_hash_function that creates a hash value for type float

// TODO: create a function of type ioopm_hash_function that creates a hash value for type void *

size_t ioopm_hash_string(elem_t key)
{
  size_t result = 0;
  // creates a hash value witth all chars in a string
  while (*key.s != '\0')
  {
    result = result * 31 + ((unsigned char)*key.s);
    key.s++;
  }
  return result;
}

// hash function compare functions
bool ioopm_int_comp(elem_t a, elem_t b)
{
    return a.i == b.i;
}

// TODO: create a function of type ioopm_eq_function that compares two unsigned int

bool ioopm_bool_comp(elem_t a, elem_t b)
{
    return a.b == b.b;
}

bool ioopm_float_comp(elem_t a, elem_t b)
{
    return a.f == b.f;
}

// TODO: create a function of type ioopm_eq_function that compares two void *

bool ioopm_string_comp(elem_t a, elem_t b)
{
    return (strcmp(a.s, b.s) == 0);
}

////////////////////////////////////////////////7
// ITERATOR
////////////////////////////////////////////////

struct hash_table_iterator
{
    ioopm_hash_table_t *ht; // hash table to iterate over
    size_t current_bucket;  // holds current bucket
    entry_t *current_entry; // holds current entry
};

/// @brief advances to the next entry, or the next bucket if the current one is empty
/// @param it iterator operated upon
static void advance_iterator_state(ioopm_hash_table_iterator_t *it)
{
    // advance to the next entry in the bucket
    it->current_entry = it->current_entry->next;

    // if it was null advance to the next bucket
    if (it->current_entry == NULL)
    {
        it->current_bucket += 1;

        // if the next bucket existed, update the current entry
        if (it->current_bucket != No_buckets)
        {
            it->current_entry = &it->ht->buckets[it->current_bucket];
        }
    }
}

/// @brief skips the sentinel nodes in the hash table
/// @param it iterator operated upon
static void skip_sentinel_nodes(ioopm_hash_table_iterator_t *it)
{
    // advances to the next entry, or the next bucket if the current one is empty
    // continues until an entry is reached, or the end of the hash table
    while (it->current_bucket != No_buckets &&
           it->current_entry == &it->ht->buckets[it->current_bucket])
    {
        advance_iterator_state(it);
    }
}

ioopm_hash_table_iterator_t *ioopm_hash_table_iterator_create(ioopm_hash_table_t *ht)
{
    // allocate memory for iterator and assign a hash table, current bucket, current entry
    ioopm_hash_table_iterator_t *it = malloc(sizeof(ioopm_hash_table_iterator_t));
    it->ht = ht;
    it->current_bucket = 0;
    it->current_entry = &ht->buckets[0];
    // skips over the sentinel nodes till it reaches an entry
    skip_sentinel_nodes(it);
    return it;
}

void ioopm_hash_table_iterator_destroy(ioopm_hash_table_iterator_t *it)
{
    free(it);
}

bool ioopm_hash_table_iterator_at_end(ioopm_hash_table_iterator_t *it)
{
    return it->current_bucket == No_buckets;
}

void ioopm_hash_table_iterator_advance(ioopm_hash_table_iterator_t *it)
{
    // advances the iterator state and skips to the next entry if it is a sentinel node
    advance_iterator_state(it);
    skip_sentinel_nodes(it);
}

elem_t ioopm_hash_table_iterator_current_key(ioopm_hash_table_iterator_t *it)
{
    return it->current_entry->key;
}

elem_t ioopm_hash_table_iterator_current_value(ioopm_hash_table_iterator_t *it)
{
    return it->current_entry->value;
}

void ioopm_hash_table_print(ioopm_hash_table_t *ht)
{
    ioopm_hash_table_iterator_t *it = ioopm_hash_table_iterator_create(ht);

    // iterates over hash table ht and puts each key paired with its corresponding value
    while (!ioopm_hash_table_iterator_at_end(it))
    {
        printf("%s:%d\n", ioopm_hash_table_iterator_current_key(it).s, ioopm_hash_table_iterator_current_value(it).i);
        ioopm_hash_table_iterator_advance(it);
    }

    ioopm_hash_table_iterator_destroy(it);
}