#include <inc/mmu.h>
#include <inc/memlayout.h>

// Page fault upcall entrypoint.

// This is where we ask the kernel to redirect us to whenever we cause
// a page fault in user space (see the call to sys_set_pgfault_handler
// in pgfault.c).
//
// When a page fault actually occurs, the kernel switches our ESP to
// point to the user exception stack if we're not already on the user
// exception stack, and then it pushes a UTrapframe onto our user
// exception stack:
//
//	48: trap-time esp
//	44: trap-time eflags
//	40: trap-time eip
//	36: utf_regs.reg_eax
//	...
//	16: utf_regs.reg_esi
//	8 : utf_regs.reg_edi
//	4 : utf_err (error code)
//	0 : utf_fault_va            <-- %esp
//
// If this is a recursive fault, the kernel will reserve for us a
// blank word above the trap-time esp for scratch work when we unwind
// the recursive call.
//
// We then have call up to the appropriate page fault handler in C
// code, pointed to by the global variable '_pgfault_handler'.

.text
.globl _pgfault_upcall
_pgfault_upcall:
	// Call the C page fault handler.
	pushl %esp			// function argument: pointer to UTF
	movl _pgfault_handler, %eax
	call *%eax
	addl $4, %esp			// pop function argument

	// Now the C page fault handler has returned and you must return
	// to the trap time state.
	// Push trap-time %eip onto the trap-time stack.
	//
	// Explanation:
	//   We must prepare the trap-time stack for our eventual return to
	//   re-execute the instruction that faulted.
	//   Unfortunately, we can't return directly from the exception stack:
	//   We can't call 'jmp', since that requires that we load the address
	//   into a register, and all registers must have their trap-time
	//   values after the return.
	//   We can't call 'ret' from the exception stack either, since if we
	//   did, %esp would have the wrong value.
	//   So instead, we push the trap-time %eip onto the *trap-time* stack!
	//   Below we'll switch to that stack and call 'ret', which will
	//   restore %eip to its pre-fault value.
	//
	//   In the case of a recursive fault on the exception stack,
	//   note that the word we're pushing now will fit in the
	//   blank word that the kernel reserved for us.
	//
	// Throughout the remaining code, think carefully about what
	// registers are available for intermediate calculations.  You
	// may find that you have to rearrange your code in non-obvious
	// ways as registers become unavailable as scratch space.
	//
	// LAB 4: Your code here.

	// Si queremos actualizar el esp primero debemos ir a la dirección 
	// donde está el utf_esp. Como se ve en el diagrama inicial
	// hay que ir 48 words hasta el tope. Lo guardo en el registro
	// comunmente utilizado como base y corro el esp 1 word para 
	// el registro eip. Actualizo que la posicion relativa
	// tenga la nueva dirección (ebx-4)
    movl 48(%esp), %ebx
	subl $0x4, %ebx
	movl %ebx, 48(%esp)
	// De manera similar, me guardo el %eip para pushearlo en el 
	// stack del usuario.
	movl 40(%esp), %eax
	movl %eax, (%ebx)
	
	// Muevo el esp 8 para que apunte a los registros
	// Luego hago el popal
	addl $0x8, %esp
	popal

	// El popal nos movio el esp a la pos 40 relativa original
	// Salteamos el eip y hacemos pop del flag
	addl $4,%esp
	popf
	
	// El popf nos movio el esp a la pos 48 relativa original
	// Ahora simplemente tengo que hacer pop del stack para restaurar
	// el stack del usuario.
	pop %esp
	
	// Finalmente, en el stack tengo el %eip cargado anteriormente
	// La instrucción ret me hace volver a la instrucción donde
	// se genero el page fault.
	ret
	
	// Restore the trap-time registers.  After you do this, you
	// can no longer modify any general-purpose registers.
	// LAB 4: Your code here.

	// Restore eflags from the stack.  After you do this, you can
	// no longer use arithmetic operations or anything else that
	// modifies eflags.
	// LAB 4: Your code here.

	// Switch back to the adjusted trap-time stack.
	// LAB 4: Your code here.

	// Return to re-execute the instruction that faulted.
	// LAB 4: Your code here.
