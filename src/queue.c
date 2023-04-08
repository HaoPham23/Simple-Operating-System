#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
	q->proc[q->size++] = proc;	
}

struct pcb_t * dequeue(struct queue_t * q) {
	if (q->size == 0)
		return NULL;

	struct pcb_t* max_default_prio_proc = q->proc[0];
	// int j = 0;

	// for (int i = 1; i < q->size; ++i)
	// 	if (max_default_prio_proc->prio > q->proc[i]->prio) {
	// 		j = i;
	// 		max_default_prio_proc = q->proc[i];
	// 	}
	
	for (int i = 0; i < q->size - 1; ++i)
		q->proc[i] = q->proc[i + 1];
	
	--q->size;
	
	return max_default_prio_proc;
}

