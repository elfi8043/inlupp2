#pragma once
#include <stddef.h>
#include "common.h"
#include <stdbool.h>

/**
 * @file hash_table.h
 * @author Elis Filén, Alexander Thoresson
 * @date 17 Sep 2026
 * @brief simple hash table that maps keys to values, both of the union type elem_t
 *
 * A hash table consists of buckets containing linked lists.
 * The amount of buckets is fixed and can not be changed.
 * At creation the hash table is empty and each bucket contains a sentinel node/NULL entry.
 * 
 */

typedef struct hash_table ioopm_hash_table_t;

/// @brief create a new hash table
/// @param hash_fn a function that takes a key and computes a hash value 
/// @param key_eq_fn a function that checks for equality between two keys
/// @return a new empty hash table
ioopm_hash_table_t *ioopm_hash_table_create(ioopm_hash_function *hash_fn, ioopm_eq_function *key_eq_fn);

/// @brief delete a hash table and free its memory
/// @param ht a hash table to be deleted
void ioopm_hash_table_destroy(ioopm_hash_table_t *ht);

/// @brief add key => value entry in hash table ht
/// @param ht hash table operated upon
/// @param key key to insert of type elem_t
/// @param value value to insert of type elem_t
void ioopm_hash_table_insert(ioopm_hash_table_t *ht, elem_t key, elem_t value);

/// @brief lookup value for key in hash table ht
/// @param ht hash table operated upon
/// @param key key to lookup
/// @param result a pointer to where the value looked up, is going to be stored
/// @return true (and the value via result mapped to by key), or false if the key does not exist
bool ioopm_hash_table_lookup(ioopm_hash_table_t *ht, elem_t key, elem_t *result);

/// @brief remove any mapping from key to a value
/// @param ht hash table operated upon
/// @param key key to remove
/// @param result a pointer to the value that was removed
/// @return the value via result mapped to by the key and boolean true if key removed, else false e.g key nonexistent
bool ioopm_hash_table_remove(ioopm_hash_table_t *ht, elem_t key, elem_t *result);

/// @brief check if hash table ht has certain key
/// @param ht hash table operated upon
/// @param key key to check
/// @return true if ht has key else false
bool ioopm_hash_table_has_key(ioopm_hash_table_t *ht, elem_t key);

/// @brief check if hash table ht is empty
/// @param ht hash table operated upon
/// @return true if ht is empty else false
bool ioopm_hash_table_is_empty(ioopm_hash_table_t *ht);

/// @brief check size of a hash table ht
/// @param ht hash table operated upon
/// @return size of the hash table
size_t ioopm_hash_table_size(ioopm_hash_table_t *ht);

/// @brief computes hash value of integer
/// @param key key to get hash value of
/// @return hash value of key
size_t ioopm_hash_int(elem_t key);

/// @brief computes hash value of string
/// @param key key to get hash value of
/// @return hash value of key
size_t ioopm_hash_string(elem_t key);

/// @brief compares if two elem_t are ints
/// @param a first variable to compare
/// @param b second variable to compare
/// @return true if a and b are both ints else false
bool ioopm_int_comp(elem_t a, elem_t b);

/// @brief compares if two elem_t are bools
/// @param a first variable to compare
/// @param b second variable to compare
/// @return true if a and b are both bools else false
bool ioopm_bool_comp(elem_t a, elem_t b);

/// @brief compares if two elem_t are floats
/// @param a first variable to compare
/// @param b second variable to compare
/// @return true if a and b are both floats else false
bool ioopm_float_comp(elem_t a, elem_t b);

/// @brief compares if two elem_t are strings
/// @param a first variable to compare
/// @param b second variable to compare
/// @return true if a and b are both strings else false
bool ioopm_string_comp(elem_t a, elem_t b);
