// Se hace fork y se ejecutan algunas iteraciones con el padre
// con mayor prioridad que el hijo. Luego se intercambian los roles
#include <inc/lib.h>

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
	
	// El hijo tiene la misma mayor prioridad (10)
	// no se debería ejecutar hasta que el padre termine
	if ((env = fork()) == 0) {
		for (i = 0; i < 5; i++) {
			sys_yield();
			cprintf("Back in environment %08x, iteration %d, cpu %d, nice %d\n",
			thisenv->env_id, i, thisenv->env_cpunum, thisenv->env_nice);
		}
		return;
	}
	// Esto se debería ejecutar de manera ininterrumpida
	for (i = 0; i < 5; i++) {
		if (i == 3) {
			r = sys_env_set_nice(thisenv->env_id, 10);
			if (r < 0) {
				panic("sys_env_set_nice: %e", r);
			}
			r = sys_env_set_nice(env, -20);
			if (r < 0) {
				panic("sys_env_set_nice: %e", r);
			}
			cprintf("Se intercambian los roles de prioridad\n");
		}
		sys_yield();
		cprintf("Back in environment %08x, iteration %d, cpu %d, nice %d\n",
		thisenv->env_id, i, thisenv->env_cpunum, thisenv->env_nice);
	}
	return;
}
