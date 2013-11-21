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
#ifndef SIMOS_H
#define SIMOS_H

#ifdef	__cplusplus
extern "C" {
#endif


/***********************************************************************
 * list declarations
 **********************************************************************/

/**
 * A node contains a void* pointer, user can insert a pointer to
 * any data wanted
 */
struct simos_list_node {
	void *data;
	struct simos_list_node *next;
	struct simos_list_node *prev;
};
typedef struct simos_list_node simos_list_node_t;

typedef struct simos_list {
	simos_list_node_t *head;
	simos_list_node_t *tail;
} simos_list_t;

/** Create a new simos_list_t* pointing to NULL. */
simos_list_t *simos_list_new();
/*============================================================================
 | All list parameters must be a valid pointer. Otherwise, bad pointer error.
 +--------------------------------------------------------------------------*/
/** Create a new node, next and prev NULL and data pointing to the parameter. */
simos_list_node_t *simos_list_new_node(void *data);
/** Check whether list is empty, but list should be a valid parameter. */
int simos_list_empty(simos_list_t *list);
/** Add a new node at tail of the list. */
void simos_list_add(simos_list_t *list, simos_list_node_t *node);
/** Find the node and delete it. Return true or false. */
int simos_list_del(simos_list_t *list, simos_list_node_t *node);
/** Delete the very first node in the list. */
simos_list_node_t *simos_list_del_head(simos_list_t *list);
/** Delete the very last node in the list. */
simos_list_node_t *simos_list_del_tail(simos_list_t *list);
/** Delete all nodes in the list AND ALSO THE DATA. */
void simos_list_free(simos_list_t *list);

void simos_list_add_after(simos_list_t *list, simos_list_node_t *node,
	simos_list_node_t *curr);

void swap_data(simos_list_node_t **a, int i, int j);

/** Generic get a member of the process in the simos_list_node_t*/
int (* p_simos_node_get_member)(simos_list_node_t *node);

/** Generic partition function */
int (* p_partition)(simos_list_node_t **a, int p, int r,
		int (* p_simos_node_get_member)(simos_list_node_t *node));

/** Get a member of the process in the simos_list_node_t*/
int get_required_execution_time(simos_list_node_t *node);
int get_remaining_time(simos_list_node_t *node);
int get_priority(simos_list_node_t *node);
int get_pid(simos_list_node_t *node);

/** Quick sort for simos_list_node */
void simos_list_sort(simos_list_t *list,
		int (* p_simos_node_get_member)(simos_list_node_t *node));
void simos_list_quick_sort(simos_list_node_t **a, int f, int l,
		int (* p_simos_node_get_member)(simos_list_node_t *node));
int simos_list_partition(simos_list_node_t **a, int p, int r,
		int (* p_simos_node_get_member)(simos_list_node_t *node));


/** Simple macro meaning: for every `var' into the `list' */
#define SIMOS_LIST_FOREACH(var, list)					\
	for ((var) = ((list)->head);					\
			(var);						\
			(var) = ((var)->next))



/***********************************************************************
 * process declarations
 **********************************************************************/
#define QTDE_PROCESS 50

/** Process information. */
typedef struct {
	/** Process' identifier. */
	int pid;
	/** Units of time required for executing the process. */
	size_t required_execution_time;
	/** Turnaround time. */
	size_t turnaround_time;
	/** Amount of memory required for the process.  */
	size_t required_memory;
	/** Memory pointer where the process was allocated in the memory. */
	size_t memory_pointer;
	/** Time at process' creation. */
	size_t entry_time;
	/** Time at process' tear down. */
	size_t out_time;

	/** Used by schedule to control the process*/
	size_t quantum;
	/** Used to sort*/
	size_t priority;
	/** Time of cpu */
	size_t running_time;
} simos_process_t;

/** Create a new process. */
simos_process_t *simos_process_create(int pid, size_t required_execution_time,
		size_t required_memory);
/** Just call free (malloc(3)) in proc. */
void simos_process_destroy(simos_process_t *proc);
/** Type cast in node->data to simos_process_t. */
simos_process_t *simos_node_to_process(simos_list_node_t *node);


/***********************************************************************
 * scheduler declararions
 **********************************************************************/
typedef struct {
	int sort_sjf;
	int sort_srtf;
	int auto_quantum;
	int priority;
} simos_scheduler_conf_t;

typedef struct {
	simos_list_t *ready;
} simos_scheduler_t;

/** Create a new ready process list. */
simos_scheduler_t *simos_scheduler_new();
/** Add a process in the list (FCFS policy). */
void simos_process_list_add(simos_list_t *proclist, simos_process_t *proc);
/** Delete the process with the refered pid. Return true or false. */
int simos_process_list_del(simos_list_t *proclist, int pid);
/** Get the process with the refered pid. Return simos_process_t*. */
simos_process_t *simos_process_list_get(simos_list_t *proclist, int pid);

    /*==========================================================*/
    /*-------- New policies must be implemented here... --------*/
    /*==========================================================*/
void simos_run(simos_list_t *ready, simos_list_t *finished,
	simos_scheduler_conf_t *conf, size_t *simostime);
void simos_run_rr(simos_list_t *ready, simos_list_t *finished,
	simos_scheduler_conf_t *conf, size_t *simostime);
void simos_run_queue(simos_list_t *ready, simos_list_t *finished,
		simos_scheduler_conf_t *conf, size_t *simostime);

simos_scheduler_conf_t *create_scheduler_conf();


/***********************************************************************
 * Memory management functions
 **********************************************************************/

#define SIMOS_BMP_SIZE 32
#define SIMOS_BMP_UNIT 4
typedef struct {
	size_t unit;
	unsigned char *map; // 8-bits
} simos_memory_bmp_t;

#define SIMOS_P 1
#define SIMOS_L 0
typedef struct {
	int s; // P or L
	size_t pointer;
	size_t blocklen;
} simos_memory_block_t;

#define QTDE_MEM 1024 //QTDE_MEM >= QTDE_PROCESS * 20
typedef struct {
	size_t size;
	unsigned char *cells;
	simos_memory_bmp_t *bmp;
	simos_list_t *lmem;
} simos_memory_t;

typedef struct {
	int bmp;
	int list;
} simos_memory_conf_t;

simos_memory_conf_t *create_memory_conf();

/** Create a memory with the refered size. */
simos_memory_t *simos_memory_create(size_t size);
/** Call free (malloc(3)) in the allocated memory. */
void simos_memory_destroy(simos_memory_t *mem);


/** bit map */
simos_memory_bmp_t *simos_memory_bmp_create(size_t size, size_t unit);
void simos_memory_bmp_destroy(simos_memory_bmp_t *bmp);
size_t simos_memory_bmp_alloc(simos_memory_t *mem, size_t blocklen);


/** list */
simos_memory_block_t *simos_memory_block_create(size_t size, size_t unit);
simos_memory_block_t *simos_node_to_memory_block(simos_list_node_t *node);
void simos_memory_list_add(simos_list_t *memlist,
		simos_memory_block_t *block);
size_t simos_memory_list_alloc(simos_memory_t *mem, size_t blocklen);


/** Generic memory allocator*/
size_t (* p_simos_memory_alloc)(simos_memory_t *mem, size_t block);

/** allocator */
size_t allocator(simos_memory_t *mem, size_t poiter, size_t block);


/**
 * Try to find a gap in the memory that fits the block size.
 *
 * Return the pointer to the base of the allocated memory. Whether is not
 *   possible to allocate, returns -1.
 */
size_t simos_memory_alloc(simos_memory_t *mem, size_t block);
/** Set memory pointed with refered length to free. */
void simos_memory_free(simos_memory_t *mem, size_t pointer, size_t length);



/***********************************************************************
 * simos declarations
 **********************************************************************/

typedef struct {
	simos_scheduler_t *sched;
	simos_memory_t *mem;
} simos_t;

/** Create a new simulation of a system with the memory size as specified. */
simos_t *simos_new(size_t memory_size);
/** Add a process to the scheduler. */
int simos_add_process(simos_t *simos, simos_process_t *proc,
		simos_memory_conf_t *mconf);
/** Execute the simulation... */
void simos_execute(simos_t *simos, simos_scheduler_conf_t *conf,
		int algorithm);
/** After execution, the memory must be freed. */
void simos_free(simos_t *simos);


#ifdef	__cplusplus
}
#endif

#endif


