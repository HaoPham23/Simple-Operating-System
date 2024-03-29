
#ifndef CPU_H
#define CPU_H

#include "common.h"

struct cpu_t {
    int cur_queue_id;
    int remaining_queue_time;
    struct pcb_t * cur_proc;
};

void init_cpu(struct cpu_t* cpu);

/* Execute an instruction of a process. Return 0
 * if the instruction is executed successfully.
 * Otherwise, return 1. */
int run(struct cpu_t * proc);

#endif

