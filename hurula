#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "linked_list.h"
#include "list_iterator.h"
#include <assert.h>

struct node
{
    elem_t val;   // holds current value
    node_t *next; // points to the next entry (possibly NULL)
};

struct list
{
    node_t *first; // points to the first entry (possibly NULL)
    node_t *last;  // points to the last entry (possibly NULL)
    int size;      // holds size of list
};

ioopm_list_t *ioopm_list_create(void)
{
    // allocate memory for linked list
    return calloc(1, sizeof(ioopm_list_t));
}

/// @brief creates a node
/// @param value value to insert
/// @param next pointer to next node
/// @return the created node
static node_t *node_create(elem_t value, node_t *next)
{
    // allocate memory for node and assigns value, next
    node_t *node = calloc(1, sizeof(node_t));
    node->val = value;
    node->next = next;
    return node;
}

void ioopm_list_destroy(ioopm_list_t *list)
{
    node_t *curr = list->first;
    // goes through the list and frees each node, and then free the list
    while (curr != NULL)
    {
        node_t *new_curr = curr->next;
        free(curr);
        curr = new_curr;
    }
    free(list);
}

/// @brief inserts node into empty list
/// @param list the linked list
/// @param value value to insert
static void empty_insert(ioopm_list_t *list, elem_t value)
{
    node_t *node = node_create(value, NULL);
    list->first = node;
    list->last = node;
    list->size += 1;
}

void ioopm_list_append(ioopm_list_t *list, elem_t value)
{
    // inserts a node if the list is empty
    if (list->size == 0)
    {
        empty_insert(list, value);
    }
    // else inserts a node last in the list
    else
    {
        node_t *node = node_create(value, NULL);
        list->last->next = node;
        list->last = node;
        list->size += 1;
    }
}

void ioopm_list_prepend(ioopm_list_t *list, elem_t value)
{
    // inserts a node if the list is empty
    if (list->size == 0)
    {
        empty_insert(list, value);
    }
    // else inserts a node first in the list
    else
    {
        list->first = node_create(value, list->first);
        list->size += 1;
    }
}

elem_t ioopm_list_head(ioopm_list_t *list)
{
    return list->first->val;
}

elem_t ioopm_list_last(ioopm_list_t *list)
{
    return list->last->val;
}

/// @brief returns the previous node
/// @param list the linked list
/// @param index position of current node
/// @return the previous node to the index
static node_t *find_previous_node(ioopm_list_t *list, int index)
{
    int count = 0;
    node_t *curr = list->first;
    while (count < (index - 1))
    {
        curr = curr->next;
        count += 1;
    }
    return curr;
}

void ioopm_list_insert(ioopm_list_t *list, int index, elem_t value)
{
    node_t *prev = find_previous_node(list, index);
    // inserts a node if the list is empty
    if (list->size == 0)
    {
        empty_insert(list, value);
    }
    // else if at the first node, inserts a node first in the list
    else if (index == 0)
    {
        list->first = node_create(value, list->first);
        list->size += 1;
        return;
    }
    // else insert a node
    else
    {
        prev->next = node_create(value, prev->next);
        list->size += 1;
    }
}

elem_t ioopm_list_remove(ioopm_list_t *list, int index)
{
    // cannot remove node if list is already empty
    if (list->size == 0)
    {
        puts("EMPTY LIST");
        return int_elem(-99);
    }
    // else if at the first node, removes the first node in the list and returns its value
    else if (index == 0)
    {
        node_t *temp = list->first;
        elem_t val = list->first->val;
        list->first = list->first->next;
        free(temp);
        list->size -= 1;
        return val;
    }
    // else if there is only one node in the list, free it and returns its value
    else if (list->size == 1)
    {
        elem_t val = list->first->val;
        free(list->first);
        return val;
    }
    // else free the node and returns its value
    elem_t val = int_elem(-1);
    node_t *prev = find_previous_node(list, index);
    val = prev->next->val;
    node_t *temp = prev->next;
    prev->next = temp->next;
    free(temp);
    list->size -= 1;
    return val;
}

elem_t ioopm_list_get(ioopm_list_t *list, int index)
{
    // cannot get node if list is empty
    if (list->size == 0)
    {
        puts("EMPTY LIST");
        return int_elem(-99);
    }
    // if at the first node, get value
    if (index == 0)
    {
        return list->first->val;
    }
    // get value at current node
    node_t *prev = find_previous_node(list, index);
    return prev->next->val;
}

int ioopm_list_size(ioopm_list_t *list)
{
    return list->size;
}

bool ioopm_list_is_empty(ioopm_list_t *list)
{
    return (list->first == NULL);
}

//////////////////////////////////////////////
//////////// ITERATOR
//////////////////////////////////////////////

struct list_iterator
{
    ioopm_list_t *list;   // list to iterate over
    node_t *current_node; // holds current node
    int curr_index;       // holds current index position
};

ioopm_list_iterator_t *ioopm_list_iterator_create(ioopm_list_t *l)
{
    // allocate memory for iterator and assigns list l, current node
    ioopm_list_iterator_t *iter = calloc(1, sizeof(ioopm_list_iterator_t));
    iter->list = l;
    iter->current_node = l->first;
    return iter;
}

void ioopm_list_iterator_destroy(ioopm_list_iterator_t *iter)
{
    free(iter);
}

bool ioopm_list_iterator_at_end(ioopm_list_iterator_t *iter)
{
    return (iter->current_node == NULL);
}

void ioopm_list_iterator_advance(ioopm_list_iterator_t *iter)
{
    iter->current_node = iter->current_node->next;
    iter->curr_index += 1;
}

elem_t ioopm_list_iterator_current(ioopm_list_iterator_t *iter)
{
    return iter->current_node->val;
}

elem_t ioopm_list_iterator_remove(ioopm_list_iterator_t *iter)
{
    return ioopm_list_remove(iter->list, iter->curr_index);
}

void ioopm_list_iterator_insert(ioopm_list_iterator_t *iter, elem_t element)
{
    // inserts node with value element into current index in list
    ioopm_list_insert(iter->list, iter->curr_index, element);
    // updates the current node in the iterator
    iter->current_node = find_previous_node(iter->list, iter->curr_index + 1);
    if (iter->list->size == 1)
    {
        iter->current_node = iter->list->first;
    }
}
