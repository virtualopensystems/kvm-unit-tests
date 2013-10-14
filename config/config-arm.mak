mach = mach-virt
iodevs = pl011 virtio_mmio
phys_base = 0x40000000

cstart.o = $(TEST_DIR)/cstart.o
bits = 32
ldarch = elf32-littlearm
kernel_offset = 0x10000
CFLAGS += -D__arm__

all: test_cases

cflatobjs += \
	lib/heap.o \
	lib/devicetree.o \
	lib/virtio.o \
	lib/virtio-testdev.o \
	lib/arm/io.o \
	lib/arm/setup.o

libeabi := lib/arm/libeabi.a
eabiobjs += \
	lib/arm/eabi_compat.o

includedirs = -I lib -I lib/libfdt

$(libcflat) $(libeabi): LDFLAGS += -nostdlib
$(libcflat) $(libeabi): CFLAGS += -ffreestanding $(includedirs)

CFLAGS += -Wextra
CFLAGS += -marm
CFLAGS += -O2
ifeq ($(PROCESSOR), $(ARCH))
	# PROCESSOR=ARCH is the default, but there is no 'arm' cpu
	CFLAGS += -mcpu=cortex-a15
else
	CFLAGS += -mcpu=$(PROCESSOR)
endif

libgcc := $(shell $(CC) -m$(ARCH) --print-libgcc-file-name)
start_addr := $(shell printf "%x\n" $$(( $(phys_base) + $(kernel_offset) )))

FLATLIBS = $(libcflat) $(LIBFDT_archive) $(libgcc) $(libeabi)
%.elf: %.o $(FLATLIBS) arm/flat.lds
	$(CC) $(CFLAGS) -nostdlib -o $@ \
		-Wl,-T,arm/flat.lds,--build-id=none,-Ttext=$(start_addr) \
		$(filter %.o, $^) $(FLATLIBS)

$(libeabi): $(eabiobjs)
	$(AR) rcs $@ $^

%.flat: %.elf
	$(OBJCOPY) -O binary $^ $@

tests-common = $(TEST_DIR)/selftest.flat

tests_and_config = $(TEST_DIR)/*.flat $(TEST_DIR)/unittests.cfg

test_cases: $(tests-common) $(tests)

$(TEST_DIR)/%.o: CFLAGS += -std=gnu99 -ffreestanding $(includedirs)

$(TEST_DIR)/selftest.elf: $(cstart.o) $(TEST_DIR)/selftest.o

arch_clean: libfdt_clean
	$(RM) $(TEST_DIR)/*.o $(TEST_DIR)/*.flat $(TEST_DIR)/*.elf \
	$(libeabi) $(eabiobjs) $(TEST_DIR)/.*.d lib/arm/.*.d
