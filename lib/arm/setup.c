#include "libcflat.h"
#include "devicetree.h"
#include "arm/sysinfo.h"
#include "heap.h"

extern void io_init(void);
extern void setup_args(char *args);

extern unsigned long stacktop;

void *mem_start;
size_t mem_size;
char *bootargs;

static void read_bootinfo(const void *fdt)
{
	int ret;

	if ((ret = dt_set(fdt)) != 0) {
		printf("setup: fdt sanity checks failed! "
		       "fdt_error: %s\n", dt_strerror(ret));
		exit(ENOEXEC);
	}

	if ((ret = dt_get_bootargs_ptr(&bootargs)) < 0) {
		printf("fdt failure: %s\n", dt_strerror(ret));
		exit(ENOEXEC);
	}

	if ((ret = dt_get_memory_params(&mem_start, &mem_size)) < 0) {
		printf("setup: can't find memory params! "
		       "fdt_error: %s\n", dt_strerror(ret));
		exit(ENOEXEC);
	}
}

void setup(unsigned long arg __unused, unsigned long id __unused,
	   const void *fdt)
{
	read_bootinfo(fdt);
	heap_init(&stacktop,
		  (unsigned long)&stacktop - (unsigned long)mem_start,
		  PAGE_SIZE);
	io_init();
	setup_args(bootargs);
}
