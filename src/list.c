/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2013 ljr-ifsp
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <malloc.h>
#include "simos.h"

/**
 * Is the list empty?
 */
int simos_list_empty(simos_list_t *list)
{
	return list->head == NULL;
}

/**
 * Allocate room for a list.
 */
simos_list_t *simos_list_new()
{
	simos_list_t *l = malloc(sizeof(simos_list_t));

	l->head = NULL;
	l->tail = NULL;

	return l;
}

/**
 * Create a new list node from data.
 */
simos_list_node_t *simos_list_new_node(void *data)
{
	simos_list_node_t *n = malloc(sizeof(simos_list_node_t));

	n->next = n->prev = NULL;
	n->data = data;

	return n;
}


/**
 * Add a node at tail of the queue.
 */
void simos_list_add(simos_list_t *list, simos_list_node_t *node)
{
	/* whether the list is empty */
	if (simos_list_empty(list)) {
		list->head = node;
		list->tail = node;
		return;
	}

	node->prev = list->tail;
	list->tail->next = node;
	list->tail = node;
}

void simos_list_add_after(simos_list_t *list, simos_list_node_t *node,
	simos_list_node_t *curr)
{
	simos_list_node_t *n;

	/* whether the list is empty */
	if (simos_list_empty(list)) {
		list->head = node;
		list->tail = node;
		return;
	}

	SIMOS_LIST_FOREACH(n, list) {
		if(n == curr) {
			node->next = curr->next;
			node->prev = curr;
			curr->next = node;

			if(node->next == NULL) {
				list->tail = node;
			}
			else {
				node->next->prev = node;
			}

			break;
		}
	}
}

/**
 * Delete an especific node in the list. Return true of false.
 */
int simos_list_del(simos_list_t *list, simos_list_node_t *node)
{
	simos_list_node_t *n;

	if (simos_list_empty(list)) {
		return 0;
	}

	SIMOS_LIST_FOREACH(n, list) {
		if (n->next == node->next && n->prev == node->prev) {
			if (n == list->head) {
				simos_list_del_head(list);
			} else if (n == list->tail) {
				simos_list_del_tail(list);
			} else {
				n->prev->next = n->next;
				n->next->prev = n->prev;
			}

			return 1;
		}
	}

	// node not found.
	return 0;
}

/**
 * Delete the first node in the list.
 */
simos_list_node_t *simos_list_del_head(simos_list_t *list)
{
	simos_list_node_t *n = list->head;

	if (simos_list_empty(list)) {
		return NULL;
	}

	if (list->head == list->tail) {
		list->head = NULL;
		list->tail = NULL;
	} else {
		list->head->next->prev = NULL;
		list->head = list->head->next;
	}

	return n;
}

/**
 * Delete the last node in the list.
 */
simos_list_node_t *simos_list_del_tail(simos_list_t *list)
{
	simos_list_node_t *n = list->tail;

	if (simos_list_empty(list)) {
		return NULL;
	}

	if (list->head == list->tail) {
		list->head = NULL;
		list->tail = NULL;
	} else {
		list->tail->prev->next = NULL;
		list->tail = list->tail->prev;
	}

	return n;
}

/**
 * Free all pointers in the list.
 */
void simos_list_free(simos_list_t *list)
{
	simos_list_node_t *n;
	SIMOS_LIST_FOREACH(n, list) {
		free(n->data);
		free(n);
	}

	list->head = NULL;
	list->tail = NULL;
}



void swap_data(simos_list_node_t **a, int i, int j)
{
	simos_list_node_t *tmp;

	a[i]->next = NULL;
	a[i]->prev = NULL;

	a[j]->next = NULL;
	a[j]->prev = NULL;

	tmp = a[i];
	a[i] = a[j];
	a[j] = tmp;
}

/** Get required_execution_time of the process in the node */
int get_required_execution_time(simos_list_node_t *node)
{
	if (node) {
		return simos_node_to_process(node)->required_execution_time;
	}

	return 0;
}

/** Get remaining_time of the process in the node */
int get_remaining_time(simos_list_node_t *node)
{
	simos_process_t *p;
	if (node) {
		p = simos_node_to_process(node);
		return p->required_execution_time - p->running_time;
	}

	return 0;
}

/** Get priority of the process in the node */
int get_priority(simos_list_node_t *node)
{
	if (node) {
		return simos_node_to_process(node)->priority;
	}

	return 0;
}

/** Get pid */
int get_pid(simos_list_node_t *node)
{
	if (node) {
		return simos_node_to_process(node)->pid;
	}

	return 0;
}

void simos_list_sort(simos_list_t *list,
		int (* p_simos_node_get_member)(simos_list_node_t *node))
{
	simos_list_node_t *n;

	size_t i = 0;
	size_t list_size = 0;

	for(n = list->head; n; n = n->next) {
		list_size++;
	}

	simos_list_node_t **array_simos_node
			= malloc(list_size * sizeof(simos_list_node_t *));

	for (n = list->head; n; n = n->next, i++) {
		array_simos_node[i] = n;
	}

	simos_list_quick_sort(array_simos_node, 0, list_size-1,
			p_simos_node_get_member);

	for (i = 0; i < (list_size - 1); i++) {
		array_simos_node[i]->next = array_simos_node[i+1];
		array_simos_node[i+1]->prev = array_simos_node[i];
	}

	list->head = array_simos_node[0];
	list->tail = array_simos_node[list_size - 1];
}

void simos_list_quick_sort(simos_list_node_t **a, int f, int l,
		int (* p_simos_node_get_member)(simos_list_node_t *node))
{
	int q;
	if(f < l) {
		q = simos_list_partition(a, f, l, p_simos_node_get_member);
		simos_list_quick_sort(a, f, q, p_simos_node_get_member);
		simos_list_quick_sort(a, q+1, l, p_simos_node_get_member);
	}
}

int simos_list_partition(simos_list_node_t **a, int p, int r,
		int (* p_simos_node_get_member)(simos_list_node_t *node))
{
	int i, j;
	size_t pivo;

	pivo = p_simos_node_get_member(a[(p+r)/2]);

	i = p-1;
	j = r+1;

	while (i < j) {
		do {
			j = j-1;
		}
		while(p_simos_node_get_member(a[j]) > pivo);

		do {
			i = i+1;
		}
		while(p_simos_node_get_member(a[i]) < pivo);

		if(i < j) {
			swap_data(a, i, j);
		}

	}
	return j;
}
