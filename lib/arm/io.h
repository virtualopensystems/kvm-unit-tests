#ifndef _ARM_IO_H_
#define _ARM_IO_H_

#define __bswap16 bswap16
static inline u16 bswap16(u16 val)
{
	u16 ret;
	asm volatile("rev16 %0, %1" : "=r" (ret) :  "r" (val));
	return ret;
}

#define __bswap32 bswap32
static inline u32 bswap32(u32 val)
{
	u32 ret;
	asm volatile("rev %0, %1" : "=r" (ret) :  "r" (val));
	return ret;
}

#include "libio.h"
#endif
