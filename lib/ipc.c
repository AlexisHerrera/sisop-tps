// User-level IPC library routines

#include <inc/lib.h>

// Receive a value via IPC and return it.
// If 'pg' is nonnull, then any page sent by the sender will be mapped at
//	that address.
// If 'from_env_store' is nonnull, then store the IPC sender's envid in
//	*from_env_store.
// If 'perm_store' is nonnull, then store the IPC sender's page permission
//	in *perm_store (this is nonzero iff a page was successfully
//	transferred to 'pg').
// If the system call fails, then store 0 in *fromenv and *perm (if
//	they're nonnull) and return the error.
// Otherwise, return the value sent by the sender
//
// Hint:
//   Use 'thisenv' to discover the value and who sent it.
//   If 'pg' is null, pass sys_ipc_recv a value that it will understand
//   as meaning "no page".  (Zero is not the right value, since that's
//   a perfectly valid place to map a page.)
int32_t
ipc_recv(envid_t *from_env_store, void *pg, int *perm_store)
{
	// LAB 4: Your code here.
	if (!pg) {
		pg = (void *) KERNBASE;  // direccion invalida
	}
	int err = sys_ipc_recv(pg);
	if (err < 0) {
		if (!from_env_store) {
			*from_env_store = 0;
		}
		if (!perm_store) {
			*perm_store = 0;
		}
		return err;
	}
	if (from_env_store) {
		*from_env_store = thisenv->env_ipc_from;
	}
	if (perm_store) {
		*perm_store = thisenv->env_ipc_perm;
	}
	// Tiene sentido que el mapeo lo haga el send ya
	// que solo se debería mapear SI se realiza el intercambio
	// es decir cuando recv está "escuchando" y send haya enviado.

	// if (!thisenv->env_ipc_dstva) {
	// 	sys_page_map(thisenv->env_ipc_from,
	// 	             (void *) thisenv->env_ipc_value,
	// 	             thisenv->env_id,
	// 	             thisenv->env_ipc_dstva,
	// 	             thisenv->env_ipc_perm);
	// }
	return thisenv->env_ipc_value;
}

// Send 'val' (and 'pg' with 'perm', if 'pg' is nonnull) to 'toenv'.
// This function keeps trying until it succeeds.
// It should panic() on any error other than -E_IPC_NOT_RECV.
//
// Hint:
//   Use sys_yield() to be CPU-friendly.
//   If 'pg' is null, pass sys_ipc_try_send a value that it will understand
//   as meaning "no page".  (Zero is not the right value.)
void
ipc_send(envid_t to_env, uint32_t val, void *pg, int perm)
{
	// LAB 4: Your code here.
	int notSent = -E_IPC_NOT_RECV;
	while (notSent) {
		if (pg) {
			notSent = sys_ipc_try_send(to_env, val, pg, perm);
		} else {
			notSent = sys_ipc_try_send(
			        to_env, val, (void *) KERNBASE, (int) NULL);
		}
		if (notSent < 0) {
			if (notSent != -E_IPC_NOT_RECV) {
				panic("try_send");
			}
			sys_yield();
		}
	}
}

// Find the first environment of the given type.  We'll use this to
// find special environments.
// Returns 0 if no such environment exists.
envid_t
ipc_find_env(enum EnvType type)
{
	int i;
	for (i = 0; i < NENV; i++)
		if (envs[i].env_type == type)
			return envs[i].env_id;
	return 0;
}
