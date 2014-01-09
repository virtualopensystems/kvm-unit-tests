#ifndef _VIRTIO_H_
#define _VIRTIO_H_
#include "libcflat.h"

#define VIRTIO_MMIO_DEVICE_ID	0x008
#define VIRTIO_MMIO_CONFIG	0x100

struct virtio_devid {
	u32 device;
	u32 vendor;
};

struct virtio_dev {
	struct virtio_devid id;
	struct virtio_conf_ops *config;
};

struct virtio_conf_ops {
	void (*get)(struct virtio_dev *vdev, unsigned offset,
		    void *buf, unsigned len);
	void (*set)(struct virtio_dev *vdev, unsigned offset,
		    const void *buf, unsigned len);
};

struct virtio_mmio_dev {
	struct virtio_dev vdev;
	void *base;
};

static inline u8
virtio_config_readb(struct virtio_dev *vdev, unsigned offset)
{
	u8 val;
	vdev->config->get(vdev, offset, &val, 1);
	return val;
}

static inline u16
virtio_config_readw(struct virtio_dev *vdev, unsigned offset)
{
	u16 val;
	vdev->config->get(vdev, offset, &val, 2);
	return val;
}

static inline u32
virtio_config_readl(struct virtio_dev *vdev, unsigned offset)
{
	u32 val;
	vdev->config->get(vdev, offset, &val, 4);
	return val;
}

static inline void
virtio_config_writeb(struct virtio_dev *vdev, unsigned offset, u8 val)
{
	vdev->config->set(vdev, offset, &val, 1);
}

static inline void
virtio_config_writew(struct virtio_dev *vdev, unsigned offset, u16 val)
{
	vdev->config->set(vdev, offset, &val, 2);
}

static inline void
virtio_config_writel(struct virtio_dev *vdev, unsigned offset, u32 val)
{
	vdev->config->set(vdev, offset, &val, 4);
}

extern struct virtio_dev *virtio_bind(u32 device);

#endif
