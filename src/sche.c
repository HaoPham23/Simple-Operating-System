
#include "queue.h"
#include "sche.h"

/*In part 2, do not include "cpu.h"*/
#include "cpu.h"
#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>
static struct queue_t ready_queue;
static struct queue_t run_queue;
static pthread_mutex_t queue_lock;

#ifdef MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];
#endif

int queue_empty(void) {
#ifdef MLQ_SCHED
	unsigned long prio;
	for (prio = 0; prio < MAX_PRIO; prio++)
		if(!empty(&mlq_ready_queue[prio])) 
			return 0;
#endif
	return (empty(&ready_queue) && empty(&run_queue));
}

void init_scheduler(void) {
    int i ;

	for (i = 0; i < MAX_PRIO; i ++)
		mlq_ready_queue[i].size = 0;
}

#ifdef MLQ_SCHED
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
#else
struct pcb_t * get_proc(void) {
	struct pcb_t * proc = NULL;
	/*TODO: get a process from [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */
	return proc;
}

void put_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&run_queue, proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&ready_queue, proc);
	pthread_mutex_unlock(&queue_lock);	
}
#endif


