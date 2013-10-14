#ifndef _ARM_SYSINFO_H_
#define _ARM_SYSINFO_H_
#include "libcflat.h"

#define PAGE_SHIFT		12
#define PAGE_SIZE		(1UL << PAGE_SHIFT)
#define PAGE_MASK		(~((1 << PAGE_SHIFT) - 1))

extern void *mem_start;
extern size_t mem_size;
extern char *bootargs;
#endif
