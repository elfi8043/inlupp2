#include "linked_list.h"
#include <stddef.h>
#include <stdlib.h>
#include "common.h"


static node_t *node_create(elem_t value)
{
    node_t *node = malloc(sizeof(node_t));
    node->value = value;
    node->next = NULL;
    return node;
}

ioopm_list_t *ioopm_list_create()
{
    /// Using calloc to reset the memory to zero. Size is 0, first and last is NULL.
    return calloc(1, sizeof(ioopm_list_t));

}

static void destroy_node(node_t *node)
{
    free(node);
}

static node_t **find_node(ioopm_list_t *list, size_t index)
{
    //Using a double pointer to avoid edge cases
    node_t **current_node = &list->first;
    for (size_t i = 0; i < index; i++)
    {
        current_node = &((*current_node)->next);
    }

    return current_node;
}

void ioopm_list_destroy(ioopm_list_t *list)
{
    size_t size = list->size;
    node_t *node_first = list->first;

    for (size_t i = 0; i < size; i++)
    {
        //Save the pointer to the next node before we free the current one
        node_t *next = node_first->next;
        destroy_node(node_first);
        node_first = next;
    }
    free(list);
}

void ioopm_list_append(ioopm_list_t *list, elem_t value)
{
    node_t *node = node_create(value);
    
    if (list->last == NULL)
    {
        list->first = node;
    }

    else
    {
        list->last->next = node;
    }
    list->last = node;
    list->size++;
}

void ioopm_list_prepend(ioopm_list_t *list, elem_t value)
{
    node_t *node = node_create(value);
    node->next = list->first;
    list->first = node;

    if (list->last == NULL)
    {
        list->last = node;
    }
    list->size++;
}

elem_t ioopm_list_head(ioopm_list_t *list)
{
    return list->first->value;
}


elem_t ioopm_list_last(ioopm_list_t *list)
{
    return list->last->value;
}

void ioopm_list_insert(ioopm_list_t *list, size_t index, elem_t value)
{
    /// Pointer to the pointer of the indexed node
    node_t **node_at_index = find_node(list, index);

    node_t *new_node = node_create(value);

    /// Make the new node point to the indexed node
    new_node->next = *node_at_index;

    /// Set the pointer of the node at index to the new node. Same as setting previous_node->next = new_node
    *node_at_index = new_node;

    if (new_node->next == NULL)
    {
        list->last = new_node;
    }
    list->size++;
}

elem_t ioopm_list_remove(ioopm_list_t *list, size_t index)
{
    node_t **pointer_to_node = find_node(list, index);
    node_t *node_to_remove = *pointer_to_node;

    //Change so the previous node points to the node after the removed one
    *pointer_to_node = node_to_remove->next;
    elem_t result = node_to_remove->value;

    if (node_to_remove == list->last)
    {
        /// If the node is the last remaining element in the list, set last to NULL.
        if (list->size == 1)
        {
            list->last = NULL;
        }

        /// If it's not, change the last node to the new last node.
        else
        {
            list->last = *find_node(list, list->size - 2);
        }
    }

    list->size--;
    destroy_node(node_to_remove);
    return result;
    
}

elem_t ioopm_list_get(ioopm_list_t *list, size_t index)
{
    node_t **pointer_to_node = find_node(list, index);
    node_t *node = *pointer_to_node;

    elem_t result = node->value;
    return result;
}

size_t ioopm_list_size(ioopm_list_t *list)
{
    return list->size;
}

bool ioopm_list_is_empty(ioopm_list_t *list)
{
    return list->size == 0;
}