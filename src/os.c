
#include "cpu.h"
#include "timer.h"
#include "sched.h"
#include "loader.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int time_slot;
static int num_cpus;
static int done = 0;

static struct ld_args{
	char ** path;
	unsigned long * start_time;
	unsigned long * prio;
} ld_processes;
int num_processes;

struct cpu_args {
	struct timer_id_t * timer_id;
	int id;
	struct cpu_t cpu;
};

static void * cpu_routine(void * args) {
	struct timer_id_t * timer_id = ((struct cpu_args*)args)->timer_id;
	int id = ((struct cpu_args*)args)->id;
	struct cpu_t* cpu = &((struct cpu_args*)args)->cpu;

	/* Check for new process in ready queue */
	int time_left = 0;
	
	while (1) {
		/* Check the status of current process */
		if (cpu->cur_proc == NULL) {
			/* No process is running, the we load new process from
		 	* ready queue */
			cpu->cur_proc = get_proc(cpu);
			if (cpu->cur_proc == NULL) {
                           next_slot(timer_id);
                           continue; /* First load failed. skip dummy load */
            }
		}else if (cpu->cur_proc->pc == cpu->cur_proc->code->size) {
			/* The porcess has finish it job */
			printf("\tCPU %d: Processed %2d has finished\n",
				id , cpu->cur_proc->pid);
			free(cpu->cur_proc);
			cpu->cur_proc = get_proc(cpu);
			time_left = 0;
		}else if (time_left == 0) {
			/* The process has done its job in current time slot */
			printf("\tCPU %d: Put process %2d to run queue\n",
				id, cpu->cur_proc->pid);
			put_proc(cpu->cur_proc);
			cpu->cur_proc = get_proc(cpu);
		} else if (cpu->remaining_queue_time <= 0) {
			/* The current queue has run out of time slot */
			time_left = 0;
			printf("\tCPU %d: Put process %2d to run queue\n",
				id, cpu->cur_proc->pid);
			put_proc(cpu->cur_proc);
			cpu->cur_proc = get_proc(cpu);
		}
		
		/* Recheck process status after loading new process */
		if (cpu->cur_proc == NULL && done) {
			/* No process to run, exit */
			printf("\tCPU %d stopped\n", id);
			break;
		}else if (cpu->cur_proc == NULL) {
			/* There may be new processes to run in
			 * next time slots, just skip current slot */
			next_slot(timer_id);
			continue;
		}else if (time_left == 0) {
			printf("\tCPU %d: Dispatched process %2d\n",
				id, cpu->cur_proc->pid);
			time_left = time_slot;
		}
		
		/* Run current process */
		run(cpu);
		--cpu->remaining_queue_time;
		time_left--;
		next_slot(timer_id);
	}
	detach_event(timer_id);
	pthread_exit(NULL);
}

static void * ld_routine(void * args) {
	struct timer_id_t * timer_id = (struct timer_id_t*)args;
	int i = 0;
	while (i < num_processes) {
		struct pcb_t * proc = load(ld_processes.path[i]);
		proc->prio = ld_processes.prio[i];
		while (current_time() < ld_processes.start_time[i]) {
			next_slot(timer_id);
		}
		printf("\tLoaded a process at %s, PID: %d PRIO: %ld\n",
			ld_processes.path[i], proc->pid, ld_processes.prio[i]);
		add_proc(proc);
		free(ld_processes.path[i]);
		i++;
		next_slot(timer_id);
	}
	free(ld_processes.path);
	free(ld_processes.start_time);
	done = 1;
	detach_event(timer_id);
	pthread_exit(NULL);
}

static void read_config(const char * path) {
	FILE * file;
	FILE * file_proc;
	if ((file = fopen(path, "r")) == NULL) {
		printf("Cannot find configure file at %s\n", path);
		exit(1);
	}
	fscanf(file, "%d %d %d\n", &time_slot, &num_cpus, &num_processes);
	ld_processes.path = (char**)malloc(sizeof(char*) * num_processes);
	ld_processes.start_time = (unsigned long*)
		malloc(sizeof(unsigned long) * num_processes);
	ld_processes.prio = (unsigned long*)
		malloc(sizeof(unsigned long) * num_processes);
	int i;
	for (i = 0; i < num_processes; i++) {
		ld_processes.path[i] = (char*)malloc(sizeof(char) * 100);
		ld_processes.path[i][0] = '\0';
		strcat(ld_processes.path[i], "input/proc/");
		char proc[100];
		// fscanf(file, "%lu %s %lu\n", &ld_processes.start_time[i], proc, &ld_processes.prio[i]);
		char line[100];
		fgets(line, sizeof(line), file);
		if (sscanf(line, "%lu %s %lu", &ld_processes.start_time[i], proc, &ld_processes.prio[i]) == 2) {
			ld_processes.prio[i] = -1;
		}
		strcat(ld_processes.path[i], proc);
		// If prio is not found, then set it equal the default priority of process
		if (ld_processes.prio[i] == -1) {
			if ((file_proc = fopen(ld_processes.path[i], "r")) == NULL) {
				printf("Cannot find process description at '%s'\n", path);
				exit(1);		
			} else {
				fscanf(file_proc, "%lu", &ld_processes.prio[i]);
				fclose(file_proc);
			}
		}
	}
}

int main(int argc, char * argv[]) {
	/* Read config */
	if (argc != 2) {
		printf("Usage: os [path to configure file]\n");
		return 1;
	}
	char path[100];
	path[0] = '\0';
	strcat(path, "input/");
	strcat(path, argv[1]);
	read_config(path);

	pthread_t * cpu_threads = (pthread_t*)malloc(num_cpus * sizeof(pthread_t));
	struct cpu_args * args =
		(struct cpu_args*)malloc(sizeof(struct cpu_args) * num_cpus);
	
	for (int i = 0; i < num_cpus; ++i)
		init_cpu(&args[i].cpu);

	pthread_t ld;
	
	/* Init timer */
	int i;
	for (i = 0; i < num_cpus; i++) {
		args[i].timer_id = attach_event();
		args[i].id = i;
	}
	struct timer_id_t * ld_event = attach_event();
	start_timer();

	/* Init scheduler */
	init_scheduler();

	/* Run CPU and loader */
	pthread_create(&ld, NULL, ld_routine, (void*)ld_event);
	for (i = 0; i < num_cpus; i++) {
		pthread_create(&cpu_threads[i], NULL,
			cpu_routine, (void*)&args[i]);
	}

	/* Wait for CPU and loader finishing */
	for (i = 0; i < num_cpus; i++) {
		pthread_join(cpu_threads[i], NULL);
	}
	pthread_join(ld, NULL);

	/* Stop timer */
	stop_timer();

	return 0;

}



