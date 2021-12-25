/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */
TRAPHANDLER_NOEC(trap_0_de, T_DIVIDE)
TRAPHANDLER_NOEC(trap_1_db, T_DEBUG)
TRAPHANDLER_NOEC(trap_2_nmi, T_NMI)
TRAPHANDLER_NOEC(trap_3_bp, T_BRKPT)
TRAPHANDLER_NOEC(trap_4_of, T_OFLOW)
TRAPHANDLER_NOEC(trap_5_br, T_BOUND)
TRAPHANDLER_NOEC(trap_6_ud, T_ILLOP)
TRAPHANDLER_NOEC(trap_7_nm, T_DEVICE)
TRAPHANDLER(trap_8_df, T_DBLFLT)
TRAPHANDLER(trap_10_ts, T_TSS)
TRAPHANDLER(trap_11_np, T_SEGNP)
TRAPHANDLER(trap_12_ss, T_STACK)
TRAPHANDLER(trap_13_gp, T_GPFLT)
TRAPHANDLER(trap_14_pf, T_PGFLT)
TRAPHANDLER_NOEC(trap_16_mf, T_FPERR)
TRAPHANDLER(trap_17_ac, T_ALIGN)
TRAPHANDLER_NOEC(trap_18_mc, T_MCHK)
TRAPHANDLER_NOEC(trap_19_xm, T_SIMDERR)
TRAPHANDLER_NOEC(trap_20_ve, 20) 
/* NO ESTA LA CONSTANTE DE ESTA PERO ESTA EN EL COSO DE INTEL */

TRAPHANDLER_NOEC(trap_48_sc, T_SYSCALL)
TRAPHANDLER(trap_500_ca, T_DEFAULT)

/* IRQs*/
TRAPHANDLER_NOEC(trap_irq_timer_0, IRQ_OFFSET+IRQ_TIMER)
TRAPHANDLER_NOEC(trap_irq_kbd, IRQ_OFFSET+IRQ_KBD)
TRAPHANDLER_NOEC(trap_irq_serial, IRQ_OFFSET+IRQ_SERIAL)



/*
 * Lab 3: Your code here for _alltraps
 */
_alltraps:
	/* Cargar los registros del tf al stack*/
	
	/* pushl se encarga del padding*/
	pushl %ds
	pushl %es
	pushal /*tf_regs*/

	/*
	 * Ahora que el stack tiene cargado el tf
	 * hay que pasarle el puntero al tf
	*/
	pushl %esp

	/* Se va a utilizar el DS del kernel*/
	mov $GD_KT, %ax
	mov %ax, %ds
	mov %ax, %es
	
	call trap
