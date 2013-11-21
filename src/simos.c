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
#include <string.h>
#include <malloc.h>
#include "simos.h"

static size_t simostime = 0;


/** Create a new simulation of a system with the memory size as specified. */
simos_t *simos_new(size_t memory_size)
{
	simos_t *s = malloc(sizeof(simos_t));

	s->sched = simos_scheduler_new();
	s->mem = simos_memory_create(memory_size);
	return s;
}

/** Add a process to the scheduler. */
int simos_add_process(simos_t *simos, simos_process_t *proc,
		simos_memory_conf_t *mconf)
{
	if(mconf->bmp) {
		p_simos_memory_alloc = simos_memory_bmp_alloc;
	} else if(mconf->list) {
		p_simos_memory_alloc = simos_memory_list_alloc;
	} else {
		p_simos_memory_alloc = simos_memory_alloc;
	}

	size_t pointer = p_simos_memory_alloc(simos->mem,
				proc->required_memory);

	if (pointer == -1) {
		return 0;
	}

	proc->memory_pointer = pointer;
	simos_process_list_add(simos->sched->ready, proc);

	return 1;
}

/** Execute the simulation... */
void simos_execute(simos_t *simos, simos_scheduler_conf_t *conf,
		int algorithm)
{
	simos_list_t *ready, *finished;
	simos_list_node_t *n;
	simos_process_t *p;
	char fname[30], algname[5];

	FILE *f;

	ready = simos->sched->ready;
	finished = simos_list_new();

	simostime = 1;

	//Schedule algorithms
	switch(algorithm) {
		case 1:
		case 2:
		case 3:
			if (conf->sort_sjf) {
				strcpy(algname, "_sjf_");
			} else if (conf->sort_srtf) {
				strcpy(algname, "_srtf_");
			} else {
				strcpy(algname, "_fcfs_");
			}
			simos_run(ready, finished, conf, &simostime);
			break;
		case 4:
			strcpy(algname, "_rr_");
			simos_run_rr(ready, finished, conf, &simostime);
			break;

		case 5:
			strcpy(algname, "_queue_");
			simos_run_queue(ready, finished, conf, &simostime);
			break;

	}

	strcpy(fname, "./out");
	strcat(fname, algname);
	strcat(fname, "%d%d%d%d.csv");
	sprintf(fname, fname, conf->sort_sjf, conf->sort_srtf,
			conf->priority, conf->auto_quantum);



	f = fopen(fname, "w+");
	fprintf(f, "pid;entry time;out time;required execution time;turnaround_time\n");
	SIMOS_LIST_FOREACH(n, finished) {
		p = simos_node_to_process(n);
		fprintf(f, "%d;%d;%d;%d;%d\n", p->pid, (int) p->entry_time,
				(int) p->out_time, (int) p->required_execution_time,
				(int) p->turnaround_time);

		//Clean finished process
		//simos_memory_free(simos->mem, p->memory_pointer,
				//p->required_memory);

		p->quantum = 1;
		p->priority = 0;
		p->running_time = 0;
	}
	fclose(f);

	//Reset conf
	conf->sort_sjf = 0;
	conf->sort_srtf = 0;
	conf->auto_quantum = 0;
	conf->priority = 0;

	simos->sched->ready = finished;
	simos_list_sort(simos->sched->ready, &get_pid);
}

/** After execution, the memory must be freed. */
void simos_free(simos_t *simos)
{
	simos_memory_destroy(simos->mem);
	simos_list_free(simos->sched->ready);
}



