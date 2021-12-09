// Se lanzan envs con distintas prioridades
// El padre que tiene la máxima prioridad va a 
// crear procesos de todas las prioridades
// y ver que se ejecutan en orden
#include <inc/lib.h>

volatile int counter;

void
umain(int argc, char **argv)
{
	envid_t env;
    int r;
	int i;
	// El proceso padre va a tener la prioridad más alta
	// Por lo tanto se va a ejecutar primero
	r = sys_env_set_nice(thisenv->env_id, -20);
	if (r < 0) {
		panic("sys_env_set_nice: %e", r);
	}
	cprintf("Hello, I am environment %08x, cpu %d, nice %d\n",
	thisenv->env_id, thisenv->env_cpunum,thisenv->env_nice);
	for (int i = -19; i < 20; i++) {
		if ((env = fork()) == 0) {
			r = sys_env_set_nice(env, i);
			if (r < 0) {
				panic("sys_env_set_nice: %e", r);
			}
			sys_yield();
			cprintf("Back in environment %08x, cpu %d, nice %d\n",
			thisenv->env_id, thisenv->env_cpunum, thisenv->env_nice);
			return;
		}
	}
	
}

