// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW 0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;
	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	// La dirección donde sucede el pagefault no está alineada.
	addr = ROUNDDOWN(addr, PGSIZE);
	pte_t pte = uvpt[PGNUM(addr)];
	// La dirección está mapeada si y solo sí el bit FEC_PR está a 1
	if (!(err & FEC_PR)) {
		panic("Pagefault por una dirección no mapeada");
	}
	// El error ocurrió por una lectura si el bit FEC_WR está a 0
	if (!(err & FEC_WR)) {
		panic("Pagefault por una lectura");
	}
	// Para verificar PTE_COW se debe usar uvpt.
	if (!(pte & PTE_COW)) {
		panic("Pagefault por una página no marcada con COW");
	}

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
	// Reservo la página en PFTEMP
	if ((r = sys_page_alloc(0, PFTEMP, PTE_U | PTE_P | PTE_W)) < 0) {
		panic("sys_page_alloc: %e", r);
	}
	// Copio lo que tengo en addr en PFTEMP
	memmove(PFTEMP, addr, PGSIZE);
	// Mapeo lo que tengo en PFTEMP a addr
	if ((r = sys_page_map(0, PFTEMP, 0, addr, PTE_U | PTE_P | PTE_W)) < 0) {
		panic("sys_page_map: %e", r);
	}
	// Borro la página temporal
	if ((r = sys_page_unmap(0, PFTEMP)) < 0) {
		panic("sys_page_unmap: %e", r);
	}
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	// LAB 4: Your code here.

	int r;
	uintptr_t addr = pn * PGSIZE;
	pte_t pte = uvpt[pn];
	// Permisos del padre menos PTE_W
	int par_perm = pte & PTE_SYSCALL;
	int perm = par_perm & (~PTE_W);
	// si tiene el permiso PTE_SHARE se comparten las paginas
	// Si tiene marcado el permiso de escritura
	// se mapea con el flag PTE_COW
	if (par_perm & PTE_SHARE) {
		if ((r = sys_page_map(0,
		                      (void *) addr,
		                      envid,
		                      (void *) addr,
		                      pte & PTE_SYSCALL)) < 0) {
			panic("sys_page_map: %e", r);
		}
	} else {  // PTE_W o PTE_COW
		perm |= PTE_COW;
		if ((r = sys_page_map(
		             0, (void *) addr, envid, (void *) addr, perm)) < 0) {
			panic("sys_page_map: %e", r);
		}
		// Si el destino tiene marcado el PTE_COW
		// remapeo la dirección con PTE_COW
		if (perm & PTE_COW) {
			if ((r = sys_page_map(
			             0, (void *) addr, 0, (void *) addr, perm)) <
			    0) {
				panic("sys_page_map: %e", r);
			}
		}
	}
	return 0;
}

static void
dup_or_share(envid_t dstenv, void *va, int perm)
{
	int r;

	// Es de escritura, se copia la página
	if (perm & PTE_W) {
		if ((r = sys_page_alloc(dstenv, va, perm)) < 0) {
			panic("sys_page_alloc: %e", r);
		}
		if ((r = sys_page_map(dstenv, va, 0, UTEMP, perm)) < 0) {
			panic("sys_page_map: %e", r);
		}
		memmove(UTEMP, va, PGSIZE);
		if ((r = sys_page_unmap(0, UTEMP)) < 0) {
			panic("sys_page_unmap: %e", r);
		}
	} else {
		// Si es de lectura, solo mapeo
		if ((r = sys_page_map(0, va, dstenv, va, perm)) < 0) {
			panic("sys_page_map: %e", r);
		}
	}
}

envid_t
fork_v0(void)
{
	// LAB 4: Your code here.
	envid_t envid;
	uintptr_t addr;
	int r;

	envid = sys_exofork();
	if (envid < 0)
		panic("fork_v0: %e", envid);
	if (envid == 0) {
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}

	for (addr = 0; addr < UTOP; addr += PGSIZE) {
		// Para ver si está mapeada, no queda otra que
		// ir al page directory del usuario y ver si
		// la dirección está mapeada.
		pde_t pde = uvpd[PDX(addr)];
		// Primero debe estar marcada como ocupada la pde
		if (pde & PTE_P) {
			// Recordar que uvpt tiene todos los ptes (1024*1024)
			// y para acceder uso el PGNUM del address (pdx+ptx)
			pte_t pte = uvpt[PGNUM(addr)];

			// Si entro acá, entonces la página está mapeada
			if (pte & PTE_P) {
				// Los permisos que va a tener
				// son los mismos que pte. Si es de escritura
				// se copia, si es de lectura se comparte.
				dup_or_share(envid,
				             (void *) addr,
				             pte & PTE_SYSCALL);
			}
		}
	}

	// Envid es el id del environment creado con sys_exofork
	// o sea, el id del environment hijo.
	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0) {
		panic("sys_env_set_status: %e", r);
	}

	return envid;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	set_pgfault_handler(pgfault);
	envid_t envid;
	uintptr_t addr;
	uint32_t pdeno, pteno;
	int r;

	envid = sys_exofork();
	if (envid < 0)
		panic("fork: %e", envid);
	// Hijo
	if (envid == 0) {
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}
	// Padre
	r = sys_page_alloc(envid,
	                   (void *) (UXSTACKTOP - PGSIZE),
	                   PTE_U | PTE_P | PTE_W);
	if (r < 0) {
		panic("fork: %e", envid);
	}
	// Instalo el manejador de exepciones en el hijo
	// El padre tiene el manejador por llamar a set_pgfault_handler
	r = sys_env_set_pgfault_upcall(envid, thisenv->env_pgfault_upcall);
	if (r < 0) {
		panic("fork: %e", envid);
	}
	// Para poder descartar una pgdir que no tiene nada mapeado
	// (o sea que (PDE & PTE_P) == 0), no podemos iterar sobre las direcciones
	// porque a priori no puedo saber si esa dirección pertenecía
	// a una pde que ya había visto que no estaba mapeada.
	// Entonces, lo más conveniente es iterar sobre el pgdir en la
	// region que mapea desde 0 a UTOP. Similar a env_free()
	for (pdeno = 0; pdeno < PDX(UTOP); pdeno++) {
		// only look at mapped page tables
		if (!(uvpd[pdeno] & PTE_P)) {
			continue;
		}
		// La pde está mapeada. Tengo que revisar toda la pte
		// para obtener las direcciones de esta region.
		for (pteno = 0; pteno < NPTENTRIES; pteno++) {
			addr = (uintptr_t) PGADDR(pdeno, pteno, 0);
			if (addr == (UXSTACKTOP - PGSIZE)) {
				continue;
			}
			// Verifico que la pte este mapeada
			if (!(uvpt[PGNUM(addr)] & PTE_P)) {
				continue;
			}
			// En vez de indicarle la dirección a copiar
			// le doy el número de la página virtual
			duppage(envid, PGNUM(addr));
		}
	}

	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0) {
		panic("sys_env_set_status: %e", r);
	}

	return envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
