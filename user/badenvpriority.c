// yield the processor to other environments

#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	envid_t env;
    int r;
	int i;
	
	cprintf("Hello, I am environment %08x, cpu %d\n",
	thisenv->env_id, thisenv->env_cpunum);
	// Se intenta darle una prioridad más alla de la posible
	r = sys_env_set_nice(thisenv->env_id, -100);
	if (r >= 0) {
		panic("sys_env_set_nice: %e", r);
	}
	// Se intenta darle una prioridad más alla de la posible
	r = sys_env_set_nice(thisenv->env_id, 100);
	if (r >= 0) {
		panic("sys_env_set_nice: %e", r);
	}
	// Se intenta darle cambiarle la prioridad a alguien que no
	// es uno mismo o su hijo
	for (size_t i = 1001; i < NENV; i++) {
		r = sys_env_set_nice(i, -20);
		if (r >= 0) {
			panic("sys_env_set_nice: %e", r);
		}
	}
	cprintf("All test passed\n");
}
