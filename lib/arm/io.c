#include "libcflat.h"
#include "libio.h"
#include "devicetree.h"
#include "virtio-testdev.h"

/*
 * Use this guess for the pl011 base in order to make an attempt at
 * having earlier printf support. We'll overwrite it with the real
 * base address that we read from the devicetree later.
 */
#define QEMU_MACH_VIRT_PL011_BASE 0x09000000UL

static volatile u8 *uart0_base = (u8 *)QEMU_MACH_VIRT_PL011_BASE;

void puts(const char *s)
{
	while (*s)
		writel(*s++, uart0_base);
}

void exit(int code)
{
	virtio_testdev_exit(code);
	halt(code);
}

void io_init(void)
{
	int node;

	node = dt_bus_find_device_compatible(&dt_default_bus, "arm,pl011");
	if (node < 0) {
		printf("can't find pl011 in device tree!\n");
		exit(ENXIO);
	}

	if (dt_bus_translate_reg(node, &dt_default_bus, 0,
				(void **)&uart0_base, NULL) < 0) {
		printf("can't set uart0_base!\n");
		exit(ENXIO);
	}

	virtio_testdev_init();
}
