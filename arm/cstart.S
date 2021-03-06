#define __ASSEMBLY__
#include "arm/asm-offsets.h"
#include "arm/ptrace.h"
#include "arm/cp15.h"

.arm

.section .init

.globl start
start:
	/*
	 * bootloader params are in r0-r2
	 * See the kernel doc Documentation/arm/Booting
	 */
	ldr	sp, =stacktop
	push	{r0-r3}

	/* set up vector table and mode stacks */
	bl	exceptions_init

	/* complete setup */
	pop	{r0-r3}
	bl	setup

	/* start the test */
	ldr	r0, =__argc
	ldr	r0, [r0]
	ldr	r1, =__argv
	bl	main
	bl	exit
	b	halt

.macro set_mode_stack mode, stack
	add	\stack, #S_FRAME_SIZE
	msr	cpsr_c, #(\mode | PSR_I_BIT | PSR_F_BIT)
	mov	sp, \stack
.endm

exceptions_init:
	mrc	p15, 0, r2, c1, c0, 0	@ read SCTLR
	bic	r2, #CR_V		@ SCTLR.V := 0
	mcr	p15, 0, r2, c1, c0, 0	@ write SCTLR
	ldr	r2, =vector_table
	mcr	p15, 0, r2, c12, c0, 0	@ write VBAR

	mrs	r2, cpsr
	ldr	r1, =exception_stacks
		/* first frame for svc mode */
	set_mode_stack	UND_MODE, r1
	set_mode_stack	ABT_MODE, r1
	set_mode_stack	IRQ_MODE, r1
	set_mode_stack	FIQ_MODE, r1
	msr	cpsr_cxsf, r2	@ back to svc mode
	mov	pc, lr

.text

.globl halt
halt:
1:	wfi
	b	1b

/*
 * Vector stubs.
 * Simplified version of the Linux kernel implementation
 *   arch/arm/kernel/entry-armv.S
 *
 * Each mode has an S_FRAME_SIZE sized stack initialized
 * in exceptions_init
 */
.macro vector_stub, name, vec, mode, correction=0
.align 5
vector_\name:
.if \correction
	sub	lr, lr, #\correction
.endif
	/*
	 * Save r0, r1, lr_<exception> (parent PC)
	 * and spsr_<exception> (parent CPSR)
	 */
	str	r0, [sp, #S_R0]
	str	r1, [sp, #S_R1]
	str	lr, [sp, #S_PC]
	mrs	r0, spsr
	str	r0, [sp, #S_PSR]

	/* Prepare for SVC32 mode. */
	mrs	r0, cpsr
	bic	r0, #MODE_MASK
	orr	r0, #SVC_MODE
	msr	spsr_cxsf, r0

	/* Branch to handler in SVC mode */
	mov	r0, #\vec
	mov	r1, sp
	ldr	lr, =vector_common
	movs	pc, lr
.endm

vector_stub 	rst,	0, UND_MODE
vector_stub	und,	1, UND_MODE
vector_stub	pabt,	3, ABT_MODE, 4
vector_stub	dabt,	4, ABT_MODE, 8
vector_stub	irq,	6, IRQ_MODE, 4
vector_stub	fiq,	7, FIQ_MODE, 4

.align 5
vector_svc:
	/*
	 * Save r0, r1, lr_<exception> (parent PC)
	 * and spsr_<exception> (parent CPSR)
	 */
	push	{ r1 }
	ldr	r1, =exception_stacks
	str	r0, [r1, #S_R0]
	pop	{ r0 }
	str	r0, [r1, #S_R1]
	str	lr, [r1, #S_PC]
	mrs	r0, spsr
	str	r0, [r1, #S_PSR]

	/*
	 * Branch to handler, still in SVC mode.
	 * r0 := 2 is the svc vector number.
	 */
	mov	r0, #2
	ldr	lr, =vector_common
	mov	pc, lr

vector_common:
	/* make room for pt_regs */
	sub	sp, #S_FRAME_SIZE
	tst	sp, #4			@ check stack alignment
	subne	sp, #4

	/* store registers r0-r12 */
	stmia	sp, { r0-r12 }		@ stored wrong r0 and r1, fix later

	/* get registers saved in the stub */
	ldr	r2, [r1, #S_R0]		@ r0
	ldr	r3, [r1, #S_R1]		@ r1
	ldr	r4, [r1, #S_PC] 	@ lr_<exception> (parent PC)
	ldr	r5, [r1, #S_PSR]	@ spsr_<exception> (parent CPSR)

	/* fix r0 and r1 */
	str	r2, [sp, #S_R0]
	str	r3, [sp, #S_R1]

	/* store sp_svc, if we were in usr mode we'll fix this later */
	add	r6, sp, #S_FRAME_SIZE
	addne	r6, #4			@ stack wasn't aligned
	str	r6, [sp, #S_SP]

	str	lr, [sp, #S_LR]		@ store lr_svc, fix later for usr mode
	str	r4, [sp, #S_PC]		@ store lr_<exception>
	str	r5, [sp, #S_PSR]	@ store spsr_<exception>

	/* set ORIG_r0 */
	mov	r2, #-1
	str	r2, [sp, #S_OLD_R0]

	/* if we were in usr mode then we need sp_usr and lr_usr instead */
	and	r1, r5, #MODE_MASK
	cmp	r1, #USR_MODE
	bne	1f
	add	r1, sp, #S_SP
	stmia	r1, { sp,lr }^

	/* Call the handler. r0 is the vector number, r1 := pt_regs */
1:	mov	r1, sp
	bl	do_handle_exception

	/* make sure we restore sp_svc and lr_svc on mode change */
	str	r6, [sp, #S_SP]
	str	lr, [sp, #S_LR]

	/* return from exception */
	msr	spsr_cxsf, r5
	ldmia	sp, { r0-pc }^

.align 5
vector_addrexcptn:
	b	vector_addrexcptn

.section .text.ex
.align 5
vector_table:
	b	vector_rst
	b	vector_und
	b	vector_svc
	b	vector_pabt
	b	vector_dabt
	b	vector_addrexcptn	@ should never happen
	b	vector_irq
	b	vector_fiq
