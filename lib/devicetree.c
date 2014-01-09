#include "libcflat.h"
#include "libfdt/libfdt.h"
#include "devicetree.h"

static const void *fdt;

const char *dt_strerror(int errval)
{
	if (errval == -EINVAL)
		return "Invalid argument";
	return fdt_strerror(errval);
}

int dt_set(const void *fdt_ptr)
{
	int ret = fdt_check_header(fdt_ptr);
	if (ret == 0)
		fdt = fdt_ptr;
	return ret;
}

const void *dt_get(void)
{
	return fdt;
}

int dt_get_bootargs_ptr(char **bootargs)
{
	const struct fdt_property *prop;
	int node, err;

	node = fdt_path_offset(fdt, "/chosen");
	if (node < 0)
		return node;

	prop = fdt_get_property(fdt, node, "bootargs", &err);
	if (prop)
		*bootargs = (char *)prop->data;
	else if (err != -FDT_ERR_NOTFOUND)
		return err;

	return 0;
}

int dt_bus_default_match(const struct dt_bus *bus __unused,
			 int nodeoffset __unused)
{
	/* just select first node found */
	return 1;
}

int dt_bus_default_translate(const struct dt_bus *bus __unused,
			     struct dt_reg *reg, void **addr, size_t *size)
{
	u64 temp64;

	if (!reg || !addr)
		return -EINVAL;

	/*
	 * default translate only understands u32 (<1> <1>) and
	 * u64 (<2> <1>|<2>) addresses
	 */
	if (reg->nr_address_cells < 1
			|| reg->nr_address_cells > 2
			|| reg->nr_size_cells < 1
			|| reg->nr_size_cells > 2)
		return -EINVAL;

	if (reg->nr_address_cells == 2)
		temp64 = ((u64)reg->address_cells[0] << 32)
					| reg->address_cells[1];
	else
		temp64 = reg->address_cells[0];

	/*
	 * If we're 32-bit, then the upper word of a two word
	 * address better be zero.
	 */
	if (sizeof(void *) == sizeof(u32) && reg->nr_address_cells > 1
			&& reg->address_cells[0] != 0)
		return -EINVAL;

	*addr = (void *)(unsigned long)temp64;

	if (size) {
		if (reg->nr_size_cells == 2)
			temp64 = ((u64)reg->size_cells[0] << 32)
						| reg->size_cells[1];
		else
			temp64 = reg->size_cells[0];

		if (sizeof(size_t) == sizeof(u32) && reg->nr_size_cells > 1
				&& reg->size_cells[0] != 0)
			return -EINVAL;

		*size = (size_t)temp64;
	}

	return 0;
}

const struct dt_bus dt_default_bus = {
	.name = "default",
	.match = dt_bus_default_match,
	.translate = dt_bus_default_translate,
};

void dt_bus_init_defaults(struct dt_bus *bus, const char *name)
{
	*bus = dt_default_bus;
	bus->name = name;
}

int dt_bus_find_device_compatible(const struct dt_bus *bus,
				  const char *compatible)
{
	int node, ret;

	if (!bus || !bus->match)
		return -EINVAL;

	node = fdt_node_offset_by_compatible(fdt, -1, compatible);

	while (node >= 0) {
		if ((ret = bus->match(bus, node)) < 0)
			return ret;
		else if (ret)
			break;
		node = fdt_node_offset_by_compatible(fdt, node, compatible);
	}

	return node;
}

static int __dt_get_num_cells(int node, u32 *address_cells, u32 *size_cells)
{
	const struct fdt_property *prop;
	u32 *data;
	int err;

	prop = fdt_get_property(fdt, node, "#address-cells", &err);
	if (!prop && err == -FDT_ERR_NOTFOUND) {

		node = fdt_parent_offset(fdt, node);
		if (node < 0)
			return node;

		return __dt_get_num_cells(node, address_cells, size_cells);

	} else if (!prop) {
		return err;
	}

	data = (u32 *)prop->data;
	*address_cells = fdt32_to_cpu(*data);

	prop = fdt_get_property(fdt, node, "#size-cells", &err);
	if (!prop) {
		printf("we can read #address-cells, but not #size-cells?\n");
		return err;
	}

	data = (u32 *)prop->data;
	*size_cells = fdt32_to_cpu(*data);

	return 0;
}

int dt_get_num_cells(int nodeoffset, u32 *address_cells, u32 *size_cells)
{
	if (!address_cells || !size_cells)
		return -EINVAL;
	return __dt_get_num_cells(nodeoffset, address_cells, size_cells);
}

int dt_get_reg(int nodeoffset, int regidx, u32 *address_cells,
	       u32 *size_cells, struct dt_reg *reg)
{
	const struct fdt_property *prop;
	u32 *data, regsz, i;
	int err;

	if (!address_cells || !size_cells || !reg)
		return -EINVAL;

	memset(reg, 0, sizeof(struct dt_reg));

	/*
	 * We assume #size-cells == 0 means translation is impossible,
	 * reserving it to indicate that we don't know what #address-cells
	 * and #size-cells are yet, and thus must try to get them from the
	 * parent.
	 */
	if (*size_cells == 0 && (err = dt_get_num_cells(nodeoffset,
					address_cells, size_cells)) < 0)
		return err;

	prop = fdt_get_property(fdt, nodeoffset, "reg", &err);
	if (prop == NULL)
		return err;

	regsz = (*address_cells + *size_cells) * sizeof(u32);

	if ((regidx + 1) * regsz > prop->len)
		return -EINVAL;

	data = (u32 *)(prop->data + regidx * regsz);

	for (i = 0; i < *address_cells; ++i, ++data)
		reg->address_cells[i] = fdt32_to_cpu(*data);
	for (i = 0; i < *size_cells; ++i, ++data)
		reg->size_cells[i] = fdt32_to_cpu(*data);

	reg->nr_address_cells = *address_cells;
	reg->nr_size_cells = *size_cells;

	return 0;
}

int __dt_bus_translate_reg(int nodeoffset, const struct dt_bus *bus,
			   int regidx, u32 *address_cells, u32 *size_cells,
			   void **addr, size_t *size)
{
	struct dt_reg reg;
	int ret;

	if (!bus || !bus->translate)
		return -EINVAL;

	ret = dt_get_reg(nodeoffset, regidx, address_cells, size_cells, &reg);
	if (ret < 0)
		return ret;

	return bus->translate(bus, &reg, addr, size);
}

int dt_bus_translate_reg(int nodeoffset, const struct dt_bus *bus,
			 int regidx, void **addr, size_t *size)
{
	/*
	 * size_cells == 0 tells dt_get_reg to get address_cells
	 * and size_cells from the parent node
	 */
	u32 address_cells, size_cells = 0;
	return __dt_bus_translate_reg(nodeoffset, bus, regidx,
			&address_cells, &size_cells, addr, size);
}

int dt_get_memory_params(void **start, size_t *size)
{
	int node = fdt_path_offset(fdt, "/memory");
	if (node < 0)
		return node;

	return dt_bus_translate_reg(node, &dt_default_bus, 0, start, size);
}
