#include "libcflat.h"

static size_t pagesize;
static void *free_head;

void heap_init(void *start, size_t size, size_t page_size)
{
	unsigned long s = (unsigned long)start;
	void *p = start;

	pagesize = page_size;

	/* page-align start of heap */
	if (s & (pagesize - 1)) {
		s += pagesize;
		s &= ~(pagesize - 1);
		p = (void *)s;
	}

	/* link free pages */
	while (size >= pagesize) {
		*(void **)p = free_head;
		free_head = p;
		p += pagesize;
		size -= pagesize;
	}
}

void *alloc_page(void)
{
	void *p;

	if (!free_head)
		return NULL;

	p = free_head;
	free_head = *(void **)free_head;
	return p;
}

void free_page(void *page)
{
	*(void **)page = free_head;
	free_head = page;
}
