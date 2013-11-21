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
#include <stdlib.h>
#include <time.h>
#include "simos.h"


/** Create a new ready process list. */
simos_scheduler_t *simos_scheduler_new()
{
	simos_scheduler_t *s = malloc(sizeof(simos_scheduler_t));

	s->ready = simos_list_new();

	return s;
}

/** Add a process in the list (FCFS policy). */
void simos_process_list_add(simos_list_t *proclist, simos_process_t *proc)
{
	simos_list_add(proclist, simos_list_new_node(proc));
}

/** private function the search a process... */
static simos_list_node_t *simos_process_list_search(simos_list_t *proclist,
		int pid)
{
	simos_list_node_t *n;
	// for each node n in the process list do:
	SIMOS_LIST_FOREACH(n, proclist) {
		if (pid == ((simos_process_t *) n->data)->pid) {
			return n;
		}
	}

	return NULL;
}

/** Delete the process with the refered pid.  Return true or false. */
int simos_process_list_del(simos_list_t *proclist, int pid)
{
	simos_list_node_t *n = simos_process_list_search(proclist, pid);

	if (n != NULL) {
		return simos_list_del(proclist, n);
	}

	return 0;
}

/** Get the process with the refered pid. Return simos_process_t*. */
simos_process_t *simos_process_list_get(simos_list_t *proclist, int pid)
{
	simos_list_node_t *p = simos_process_list_search(proclist, pid);

	if (p != NULL) {
		return ((simos_process_t *) p->data);
	}

	return NULL;
}

/** Create a new scheduler conf */
simos_scheduler_conf_t *create_scheduler_conf()
{
	simos_scheduler_conf_t *c = malloc(sizeof(simos_scheduler_conf_t));

	c->sort_sjf = 0;
	c->sort_srtf = 0;
	c->priority = 0;
	c->auto_quantum = 0;

	return c;
}

/*****************************************************************
 * First-Come, First-Served (FCFS)
 * Shortest-Job-First (SJF)
 * Shortest-Remaning-Time-First (SRTF)
 ****************************************************************/
void simos_run(simos_list_t *ready, simos_list_t *finished,
		simos_scheduler_conf_t *conf, size_t *simostime)
{
	simos_list_node_t *n;
	simos_process_t *p;

	// set entry time and running time
	SIMOS_LIST_FOREACH(n, ready) {
		p = simos_node_to_process(n);
		p->entry_time = *simostime;
		p->running_time = *simostime;

		(*simostime)++;
	}

	// FCFS else SJF else SRTF
	if(conf->sort_sjf) {
		simos_list_sort(ready, &get_required_execution_time);
	} else if(conf->sort_srtf) {
		simos_list_sort(ready, &get_remaining_time);
	}

	// for each node in scheduler's ready list do:
	SIMOS_LIST_FOREACH(n, ready) {
		printf("Current time is: %6ld - ", *simostime);
		

		/* time accounting */
		p = simos_node_to_process(n);
		printf("Process need %4d time. ",
				(int) p->required_execution_time);
		(*simostime) += p->required_execution_time;
		p->out_time = *simostime;
		p->running_time = *simostime;
		printf("Process %2d leaving at %ld.\n", p->pid, *simostime);
		

		// Move process from read to finished
		p->turnaround_time = p->out_time - p->entry_time;
		simos_list_add(finished, n);
		simos_list_del(ready, n);
	}
}

void funcr(simos_list_t *ready, simos_list_t *finished,
		simos_scheduler_conf_t *conf, size_t *simostime, int qnum)
{
	simos_list_node_t *n;
	simos_process_t *p;

	int timeint = 0;
	int cpuint = 0;
	int node_finished = 0;
	
	// for each node in scheduler's ready list do:
	SIMOS_LIST_FOREACH(n, ready) {
		printf("Current time is: %6ld - ", *simostime);


		/* time accounting */
		p = simos_node_to_process(n);
		printf("Process %2d. Quantum %1d. Priority %1d. ",
			p->pid,
			(int) p->quantum,
			(int) p->priority);
		
		printf("Queue %d. ", qnum);
		while ((! cpuint) && (! node_finished)) {
			(*simostime)++; //system time
			timeint++; //cpu time int

			p->running_time++;
			node_finished = p->required_execution_time <= p->running_time;
			if(conf->priority) {
				cpuint = !(timeint%(20*(6-p->priority)));
			} else {
				cpuint = !(timeint%(50*p->quantum)); //Int 50Hz * quantum
			}
		}

		printf("Need %4d time. Running at %4d. ",
				(int) p->required_execution_time,
				(int) p->running_time);
		if (cpuint) {
			printf("Interrupt %4ld. ", *simostime);
			cpuint = 0;
		}
		timeint = 0;

		if(node_finished) {
			printf("[ Finished at %4d ]",
					(int) p->running_time);
			node_finished = 0;
			p->out_time = *simostime;

			//Move process from read to finished
			p->turnaround_time = p->out_time - p->entry_time;
			simos_list_del(ready, n);
			simos_list_add(finished, n);
			if (simos_list_empty(ready)) {
				n->next = NULL;
			}
		}
		printf("\n");
	}
}

void simos_run_rr(simos_list_t *ready, simos_list_t *finished,
		simos_scheduler_conf_t *conf, size_t *simostime)
{
	simos_list_node_t *n;
	simos_process_t *p;
	
	int has_node = 1;

	srand(time(NULL));
	SIMOS_LIST_FOREACH(n, ready) {
		p = simos_node_to_process(n);
		p->entry_time = *simostime;
		(*simostime)++;

		if(conf->auto_quantum){
			if (! (p->required_execution_time%2)) {
				p->quantum = 2;
			}
		}

		if(conf->priority) {
			p->priority = rand() % 6; //6 Priority: 0-5
		}
	}

	if(conf->priority) {
		simos_list_sort(ready, &get_priority);
	} else {
		if (conf->sort_sjf) {
			simos_list_sort(ready, &get_required_execution_time);
		}
	}

	while (has_node) {
		funcr(ready, finished, conf, simostime, 0);
		has_node = !simos_list_empty(ready);
	}
}

void simos_run_queue(simos_list_t *ready, simos_list_t *finished,
		simos_scheduler_conf_t *conf, size_t *simostime)
{
	simos_list_node_t *n;
	simos_process_t *p;

	simos_list_t *q0;
	simos_list_t *q1;

	int q0exec = 2;
	int q1exec = 1;
	int has_node = 0;

	q0 = simos_list_new();
	q1 = simos_list_new();

	//select queue by 1 or 2
	n = ready->head;
	while(n) {
		has_node = 1;

		p = simos_node_to_process(n);
		p->entry_time = *simostime;
		(*simostime)++;

		ready->head = n->next;
		if (ready->head) {
			ready->head->prev = NULL;
		}

		n->prev = NULL;
		n->next = NULL;
		if (p->required_execution_time%2) {
			simos_list_add(q1, n);
		} else {
			simos_list_add(q0, n);
		}

		n = ready->head;
	}

	if(conf->sort_sjf) {
		simos_list_sort(q0, &get_required_execution_time);
		simos_list_sort(q1, &get_required_execution_time);
	}

	while (has_node) {
		while (q0exec) {
			funcr(q0, finished, conf, simostime, 0);
			q0exec--;
		}
		q0exec = 2;

		while (q1exec) {
			funcr(q1, finished, conf, simostime, 1);
			q1exec--;
		}
		q1exec = 1;

		has_node = (! simos_list_empty(q0))
					|| (! simos_list_empty(q1));
	}
}
