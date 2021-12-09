#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

void sched_halt(void);

// Choose a user environment to run and run it.
void
sched_yield(void)
{
	struct Env *idle;

	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running.  Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// LAB 4: Your code here.
	size_t index_start = 0;
	size_t idx_last_env = 0;
	size_t idx_run = -1;
	if (curenv != NULL) {
		idx_last_env = ENVX(curenv->env_id);
		index_start = idx_last_env + 1;
		if (envs[idx_last_env].env_status == ENV_RUNNING) {
			idx_run = idx_last_env;
		}
	}
	// Si es el primer proceso, se inicia desde 0 hasta NENV
	// Sino va desde idx_last_env + 1
	for (size_t i = index_start; i < NENV; i++) {
		if (envs[i].env_status == ENV_RUNNABLE) {
			if (idx_run == -1 ||
			    envs[i].env_nice < envs[idx_run].env_nice) {
				idx_run = i;
			}
		}
	}
	// Si se empezó desde el idx_last_env + 1, faltan ver
	// los envs desde 0 hasta idx_last_env

	for (size_t i = 0; i < idx_last_env; i++) {
		if (envs[i].env_status == ENV_RUNNABLE) {
			if (idx_run == -1 ||
			    envs[i].env_nice < envs[idx_run].env_nice) {
				idx_run = i;
			}
		}
	}

	// Si se obtuvo un env que se pueda correr,
	// utilizamos el que menor nice tenga.
	// Si el env que llamo a sched_yield sigue corriendo
	// y no se encontró otro con menor nice, entonces vuelve a correr.
	if (idx_run != -1) {
		env_run(&envs[idx_run]);
	}
	// sched_halt never returns
	sched_halt();
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}
