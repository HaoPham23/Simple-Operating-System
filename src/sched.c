
#include "queue.h"
#include "sched.h"
#include "cpu.h"
#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>

#define MAX_PRIO 140

static pthread_mutex_t queue_lock;

static struct queue_t mlq_ready_queue[MAX_PRIO];

int queue_empty(void) {
	unsigned long prio;
	for (prio = 0; prio < MAX_PRIO; prio++)
		if(!empty(&mlq_ready_queue[prio])) 
			return 0;
	return 1;
}

void init_scheduler(void) {
    int i ;

	for (i = 0; i < MAX_PRIO; i ++)
		mlq_ready_queue[i].size = 0;
}

/* 
 *  Stateful design for routine calling
 *  based on the priority and our MLQ policy
 *  We implement stateful here using transition technique
 *  State representation   prio = 0 .. MAX_PRIO, curr_slot = 0..(MAX_PRIO - prio)
 */

struct pcb_t * get_mlq_proc(struct cpu_t* cpu) {
	struct pcb_t * proc = NULL;
	pthread_mutex_lock(&queue_lock);

	if (queue_empty()) {
		proc = NULL;
	}
	else {
		if (cpu->remaining_queue_time <= 0) {
			cpu->cur_queue_id = (cpu->cur_queue_id + 1) % MAX_PRIO;
			cpu->remaining_queue_time = MAX_PRIO - cpu->cur_queue_id;
		}

		while (empty(&mlq_ready_queue[cpu->cur_queue_id])) {	
			cpu->cur_queue_id = (cpu->cur_queue_id + 1) % MAX_PRIO;
			cpu->remaining_queue_time = MAX_PRIO - cpu->cur_queue_id;
		}

		proc = dequeue(&mlq_ready_queue[cpu->cur_queue_id]);
	}

	pthread_mutex_unlock(&queue_lock);
	return proc;	
}

void put_mlq_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_mlq_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);	
}

struct pcb_t * get_proc(struct cpu_t* cpu) {
	return get_mlq_proc(cpu);
}

void put_proc(struct pcb_t * proc) {
	return put_mlq_proc(proc);
}

void add_proc(struct pcb_t * proc) {
	return add_mlq_proc(proc);
}