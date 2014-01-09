#ifndef _VIRTIO_TESTDEV_H_
#define _VIRTIO_TESTDEV_H_
#include "libcflat.h"

extern void virtio_testdev_init(void);
extern void virtio_testdev_version(u32 *version);
extern void virtio_testdev_clear(void);
extern void virtio_testdev_exit(int code);
#endif
