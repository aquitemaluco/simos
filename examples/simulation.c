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
#include <simos.h>
#include <time.h>

int main(int argc, char *argv[])
{
	int i, rtime, rmem;
	int opt, optconf;
	simos_list_node_t *n;
	simos_process_t *p;
	simos_t *s = simos_new(QTDE_MEM);
	simos_scheduler_conf_t *conf = create_scheduler_conf();
	simos_memory_conf_t *mconf = create_memory_conf();
	simos_process_t *proc;

	printf("Gerenciar memória com:\nMapa de bit? 1 = SIM / 0 = NAO ");
	scanf("%1d", &optconf);
	if(optconf){
		mconf->bmp = 1;
	} else {
		printf("Lista? 1 = SIM / 0 = NAO ");
		scanf("%1d", &optconf);
		if(optconf){
			mconf->list = 1;
		}
	}

	if (mconf->bmp) {
		//need plug aditional in simos to manager with bit map

		/* ***********************
		 * 4 units each bit
		 * 8 bits each row of map
		 * 1024 memory size
		 * 1024/8/4 = 32
		 *************************/
		s->mem->bmp = simos_memory_bmp_create(SIMOS_BMP_SIZE,
			SIMOS_BMP_UNIT);
	} else if (mconf->list) {
		s->mem->lmem = simos_list_new();

		//default block
		simos_memory_list_add(s->mem->lmem,
			simos_memory_block_create(0, QTDE_MEM));
	}


	srand(time(NULL));
	for (i = 0; i < QTDE_PROCESS; i++) {
		rtime = rand() % 500;
		if (rtime < QTDE_PROCESS) {
			rtime = rtime + QTDE_PROCESS;
		}

		rmem = rand() % 10;
		rmem += 10; // >=10 and <20

		proc = simos_process_create(i, rtime, rmem);
		simos_add_process(s,proc, mconf);
		printf("process %2d created\n", i);
	}
	printf("\n\n");

	for(;;){
		printf(" +===================================================+\n");
		printf(" |      --Entre com a opcao de escalonamento--       |\n");
		printf(" +===================================================+\n");
		printf(" | 1 - First-Come, First-Served (FCFS)               |\n");
		printf(" | 2 - Shortest-Job-First (SJF)                      |\n");
		printf(" | 3 - Shortest-Remaning-Time-First (SRTF)           |\n");
		printf(" | 4 - Round-Robin                                   |\n");
		printf(" | 5 - Fila prioridade                               |\n");
		printf(" | 0 - Sair                                          |\n");
		printf(" |                                                   |\n");
		printf(" +===================================================+\n");
		printf("  Opcao:");
		scanf("%1d", &opt);

		if (opt < 1 || opt > 9) break;

		switch(opt) {
			case 1: //FCFS
				simos_execute(s, conf, 1);
				break;
			case 2: //SJF
				conf->sort_sjf = 1;
				simos_execute(s, conf, 2);
				break;
			case 3: //SRTF
				conf->sort_srtf = 1;
				simos_execute(s, conf, 3);
				break;


			case 4: //RR
				printf("Quanta diferentes? 1 = SIM / 0 = Nao ");
				scanf("%1d", &optconf);
				if(optconf){
					conf->auto_quantum = 1;
				} else {
					printf("Com prioridade? 1 = SIM / 0 = Nao ");
					scanf("%1d", &optconf);
					if(optconf){
						conf->priority = 1;
					} else {
						printf("SJF? 1 = SIM / 0 = Nao ");
						scanf("%1d", &optconf);
						if(optconf){
							conf->sort_sjf = 1;
						}
					}
				}

				simos_execute(s, conf, 4);
				break;

			case 5: //queue
				printf("SJF? 1 = SIM / 0 = Nao ");
				scanf("%1d", &optconf);
				if(optconf){
					conf->sort_sjf = 1;
				}
				simos_execute(s, conf, 5);
				break;
		}
	}
	simos_free(s);
	return 0;
}

