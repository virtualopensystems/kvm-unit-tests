#include "libcflat.h"
#include "devicetree.h"
#include "libio.h"
#include "heap.h"
#include "virtio.h"

struct virtio_dt_device_data {
	u32 device;
	struct dt_bus *bus;
	int node;
	/* cached variables */
	u32 address_cells, size_cells;
};

static void virtio_dt_device_data_init(struct virtio_dt_device_data *data,
				       u32 device, struct dt_bus *bus)
{
	memset(data, 0, sizeof(struct virtio_dt_device_data));
	data->device = device;
	data->bus = bus;
}

static int vm_dt_bus_match(const struct dt_bus *bus, int nodeoffset);
static struct virtio_dev *vm_bind(struct virtio_dt_device_data *data);

struct virtio_dev *virtio_bind(u32 device)
{
	struct virtio_dt_device_data data;
	struct dt_bus bus;
	const char *compatible;

	/*
	 * currently we only support virtio-mmio
	 */
	compatible = "virtio,mmio";
	dt_bus_init_defaults(&bus, compatible);
	bus.match = vm_dt_bus_match;
	bus.private = (void *)&data;
	virtio_dt_device_data_init(&data, device, &bus);

	data.node = dt_bus_find_device_compatible(&bus, compatible);
	if (data.node < 0) {
		printf("virtio bind for device id 0x%x failed, "
		       "fdt_error: %s\n", device, dt_strerror(data.node));
		return NULL;
	}

	return vm_bind(&data);
}

static int virtio_dt_bus_translate_reg(int nodeoffset,
				       const struct dt_bus *bus,
				       int regidx, void **addr,
				       size_t *size)
{
	struct virtio_dt_device_data *data =
		(struct virtio_dt_device_data *)bus->private;

	return __dt_bus_translate_reg(nodeoffset, bus, regidx,
			&data->address_cells, &data->size_cells, addr, size);
}

/******************************************************
 * virtio_mmio
 ******************************************************/

static int vm_dt_bus_match(const struct dt_bus *bus, int nodeoffset)
{
	struct virtio_dt_device_data *data =
		(struct virtio_dt_device_data *)bus->private;
	void *addr;
	int ret;

	ret = virtio_dt_bus_translate_reg(nodeoffset, bus, 0, &addr, NULL);
	if (ret < 0) {
		printf("can't get reg! fdt_error: %s\n", dt_strerror(ret));
		return ret;
	}

	return readl(addr + VIRTIO_MMIO_DEVICE_ID) == data->device;
}

#define to_virtio_mmio_dev(vdev_ptr) \
	container_of(vdev_ptr, struct virtio_mmio_dev, vdev)

static void vm_get(struct virtio_dev *vdev, unsigned offset,
		   void *buf, unsigned len)
{
	struct virtio_mmio_dev *vmdev = to_virtio_mmio_dev(vdev);
	u8 *p = buf;
	unsigned i;

	for (i = 0; i < len; ++i)
		p[i] = readb(vmdev->base + VIRTIO_MMIO_CONFIG + offset + i);
}

static void vm_set(struct virtio_dev *vdev, unsigned offset,
		   const void *buf, unsigned len)
{
	struct virtio_mmio_dev *vmdev = to_virtio_mmio_dev(vdev);
	const u8 *p = buf;
	unsigned i;

	for (i = 0; i < len; ++i)
		writeb(p[i], vmdev->base + VIRTIO_MMIO_CONFIG + offset + i);
}

static struct virtio_dev *vm_bind(struct virtio_dt_device_data *data)
{
	struct virtio_mmio_dev *vmdev;
	void *page;

	page = alloc_page();
	vmdev = page;
	vmdev->vdev.config = page + sizeof(struct virtio_mmio_dev);

	vmdev->vdev.id.device = data->device;
	vmdev->vdev.id.vendor = -1;
	vmdev->vdev.config->get = vm_get;
	vmdev->vdev.config->set = vm_set;

	(void)virtio_dt_bus_translate_reg(data->node, data->bus, 0,
						&vmdev->base, NULL);

	return &vmdev->vdev;
}
