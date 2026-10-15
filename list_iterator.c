#include "linked_list.h"
#include "list_iterator.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

struct list_iterator
{
  ioopm_list_t *list;
  size_t steps;
  node_t *current_node;
  size_t size;
};

ioopm_list_iterator_t *ioopm_list_iterator_create(ioopm_list_t *l)
{
    ioopm_list_iterator_t *iter = malloc(sizeof(ioopm_list_iterator_t));
    iter->list = l;
    iter->current_node = l->first;
    iter->steps = 0;
    iter->size = ioopm_list_size(l);
    return iter;
}


void ioopm_list_iterator_destroy(ioopm_list_iterator_t *iter)
{
    free(iter);
}

bool ioopm_list_iterator_at_end(ioopm_list_iterator_t *iter)
{
    if (iter->steps == iter->size) return true;
    else return false;
}

void ioopm_list_iterator_advance(ioopm_list_iterator_t *iter)
{
    node_t *next = iter->current_node->next;
    iter->current_node = next;
    iter->steps++;
}

elem_t ioopm_list_iterator_current(ioopm_list_iterator_t *iter)
{
    return iter->current_node->value;
}