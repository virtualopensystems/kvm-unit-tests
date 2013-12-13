#ifndef _ARM_PROCESSOR_H_
#define _ARM_PROCESSOR_H_
#include "libcflat.h"
#include "ptrace.h"

enum vector {
	EXCPTN_RST,
	EXCPTN_UND,
	EXCPTN_SVC,
	EXCPTN_PABT,
	EXCPTN_DABT,
	EXCPTN_ADDREXCPTN,
	EXCPTN_IRQ,
	EXCPTN_FIQ,
	EXCPTN_MAX,
};

typedef void (*exception_fn)(struct pt_regs *);
extern void install_exception_handler(enum vector v, exception_fn fn);

extern void show_regs(struct pt_regs *regs);
extern void *get_sp(void);

extern void start_usr(void (*func)(void));

static inline unsigned long current_cpsr(void)
{
	unsigned long cpsr;
	asm volatile("mrs %0, cpsr" : "=r" (cpsr));
	return cpsr;
}

#define current_mode() (current_cpsr() & MODE_MASK)

#endif
