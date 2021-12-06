// test user-level fault handler -- alloc pages to fix faults
// doesn't work because we sys_cputs instead of cprintf (exercise: why?)
// Porque cputs también hace una verificación de que el proceso, tenga
// permisos para acceder a la región de memoria 0xDEADBEEF
// y luego ejecuta cprintf. Como esto no sucede, se lanza la exepción
// pagefault, que debería handlearla esta función.
#include <inc/lib.h>

void
handler(struct UTrapframe *utf)
{
	int r;
	void *addr = (void*)utf->utf_fault_va;

	cprintf("fault %x\n", addr);
	if ((r = sys_page_alloc(0, ROUNDDOWN(addr, PGSIZE),
				PTE_P|PTE_U|PTE_W)) < 0)
		panic("allocating at %x in page fault handler: %e", addr, r);
	snprintf((char*) addr, 100, "this string was faulted in at %x", addr);
}

void
umain(int argc, char **argv)
{
	set_pgfault_handler(handler);
	sys_cputs((char*)0xDEADBEEF, 4);
}
