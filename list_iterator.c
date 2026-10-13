#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "linked_list.h"
#include "list_iterator.h"

struct list_iterator
{
    node_t *current_node;
};

ioopm_list_iterator_t *ioopm_list_iterator_create(ioopm_list_t *l)
{
    return calloc(1, sizeof(ioopm_list_iterator_t));
}

void ioopm_list_iterator_destroy(ioopm_list_iterator_t *iter);
{
    free(iter);
}

bool ioopm_list_iterator_at_end(ioopm_list_iterator_t *iter)
{
    return (iter->current_node->next = NULL);
}

void ioopm_list_iterator_advance(ioopm_list_iterator_t *iter);

int ioopm_list_iterator_current(ioopm_list_iterator_t *iter);

/// NOTE: REMOVE IS OPTIONAL TO IMPLEMENT
/// @brief Remove the current element from the underlying list
/// @param iter the iterator
/// @return the removed element
int ioopm_list_iterator_remove(ioopm_list_iterator_t *iter);

/// NOTE: INSERT IS OPTIONAL TO IMPLEMENT
/// @brief Insert a new element into the underlying list making the current element it's next
/// @param iter the iterator
/// @param element the element to be inserted
void ioopm_list_iterator_insert(ioopm_list_iterator_t *iter, int element);