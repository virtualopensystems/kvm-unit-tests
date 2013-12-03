#ifndef _HEAP_H_
#define _HEAP_H_
#include "libcflat.h"

extern void heap_init(void *start, size_t size, size_t page_size);
extern void *alloc_page(void);
extern void free_page(void *page);
#endif
