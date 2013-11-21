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
#include <stdio.h>
#include <malloc.h>
#include "simos.h"

/** Create a memory with the refered size. */
simos_memory_t *simos_memory_create(size_t size)
{

    simos_memory_t *mem = malloc(sizeof(simos_memory_t));

    mem->size = size;
    mem->cells = calloc(size, sizeof(unsigned char)); // set memory to zero...

    return mem;
}

/** Call free (malloc(3)) in the allocated memory. */
void simos_memory_destroy(simos_memory_t *mem)
{
    free(mem->cells);
    free(mem);
}

/** Create a bit map to manager the free memory. */
simos_memory_bmp_t *simos_memory_bmp_create(size_t size, size_t unit)
{

    simos_memory_bmp_t *bmp = malloc(sizeof(simos_memory_bmp_t));

    bmp->unit = unit;
    bmp->map = calloc(size, sizeof(unsigned char));

    return bmp;
}

void simos_memory_bmp_destroy(simos_memory_bmp_t *bmp)
{
    free(bmp->map);
    free(bmp);
}

/** Create a new memory conf */
simos_memory_conf_t *create_memory_conf()
{
	simos_memory_conf_t *c = malloc(sizeof(simos_memory_conf_t));

	c->bmp = 0;
	c->list = 0;

	return c;
}


/** Create a list to manager the free memory. */
simos_memory_block_t *simos_memory_block_create(size_t pointer,
		size_t blocklen)
{
    simos_memory_block_t *block = malloc(sizeof(simos_memory_block_t));

    block->s = SIMOS_L;
    block->pointer = pointer;
    block->blocklen = blocklen;

    return block;
}

/** Type cast in node->data to simos_memory_block_t. */
simos_memory_block_t *simos_node_to_memory_block(simos_list_node_t *node)
{
	return ((simos_memory_block_t *) node->data);
}

void simos_memory_list_add(simos_list_t *memlist, simos_memory_block_t *block)
{
	simos_list_add(memlist, simos_list_new_node(block));
}

size_t simos_memory_list_alloc(simos_memory_t *mem, size_t blocklen)
{
	int pointer = 0;
	int found = 0;
	simos_list_node_t *n, *nn;
	simos_memory_block_t *b, *bb;
	SIMOS_LIST_FOREACH(n, mem->lmem) {
		b = simos_node_to_memory_block(n);
		if (b->s == SIMOS_L && b->blocklen > blocklen) {
			found = 1;
			break;
		}
	}

	if (found) {
		pointer = allocator(mem, b->pointer, blocklen);

		bb = simos_memory_block_create(b->pointer+blocklen, b->blocklen-blocklen);
		nn = simos_list_new_node(bb);
		simos_list_add_after(mem->lmem, nn, n);

		b->s = SIMOS_P;
		b->blocklen = blocklen;
	}

	return pointer;
}




/** allocator */
size_t allocator(simos_memory_t *mem, size_t pointer, size_t block)
{
	int i;
	for (i = pointer; i < (pointer + block); i++) {
		mem->cells[i] = 1;
	}

	printf("poiter = %4d block = %4d ", (int) pointer, (int) block);
	return pointer;
}

size_t simos_memory_alloc(simos_memory_t *mem, size_t block)
{
    int i = 0, available = 0, pointer;

    /* try to find how many gaps existing in the memory */
    while (1) {
        /* find a gap in the memory */
        while (i < mem->size && mem->cells[i] != 0) {
            i++;
        }
        //printf("[MEM] Gap found at %d\n", i);

        /* measure gap's size */
        while (i < mem->size && available < block && mem->cells[i] == 0) {
            i++;
            available++;
        }

        if (available == block) {
            pointer = i - block;
            allocator(mem, pointer, block);
            return pointer;
        } else if (i > mem->size) {
            //printf("Not enough space... No memory allocated.");
            return -1;
        }
    }

}

size_t simos_memory_bmp_alloc(simos_memory_t *mem, size_t block)
{
	simos_memory_bmp_t *bmp = mem->bmp;

	int i = 0, j = 0;
	int pointer = -1;
	int curr, prev = 0;
	int currval = 0, preval = 0;
	unsigned char control;

	int z;
	for(z=0; (z<SIMOS_BMP_SIZE) && (j<block); z++) {
		curr = z;
		prev = (curr > 0) ? curr-1 : 0;
		preval = currval;
		currval = 0;
		control = 1;
		for(;control && (j<block);) {
			if(!(bmp->map[curr] & control)) {

				if (pointer == -1) {
					pointer = i;
					j = bmp->unit;
				} else {
					j += bmp->unit ;
				}

				currval += control;
			} else {
				pointer = -1;
				if (currval > 0) currval = 0;
				if (prev > 0)  preval = 0;
			}

			control = control << 1;
			i++;
		}
	}

	pointer = allocator(mem, pointer*(bmp->unit), block);
	bmp->map[curr] += currval;
	bmp->map[prev] += preval;

	return pointer;
}

/** Set memory pointed with refered length to free. */
void simos_memory_free(simos_memory_t *mem, size_t pointer, size_t length)
{
    int i;
    for (i = pointer; i < (pointer + length); i++) {
        mem->cells[i] = 0;
    }
}


