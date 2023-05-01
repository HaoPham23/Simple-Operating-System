#ifndef SCHED_H
#define SCHED_H

#include "cpu.h"
#include "common.h"

#ifndef MLQ_SCHED
#define MLQ_SCHED
#endif

int queue_empty(void);

void init_scheduler(void);
void finish_scheduler(void);

/* Get the next process from ready queue */
struct pcb_t * get_proc(struct cpu_t *);

/* Put a process back to run queue */
void put_proc(struct pcb_t * proc);

/* Add a new process to ready queue */
void add_proc(struct pcb_t * proc);

#endif