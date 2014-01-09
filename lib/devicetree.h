#ifndef _DEVICETREE_H_
#define _DEVICETREE_H_
#include "libcflat.h"

/*
 * set/get the fdt pointer
 */
extern int dt_set(const void *fdt_ptr);
extern const void *dt_get(void);

/*
 * bootinfo accessors
 */
extern int dt_get_bootargs_ptr(char **bootargs);
extern int dt_get_memory_params(void **start, size_t *size);

#define MAX_ADDRESS_CELLS	4
#define MAX_SIZE_CELLS		4
struct dt_reg {
	u32 nr_address_cells;
	u32 nr_size_cells;
	u32 address_cells[MAX_ADDRESS_CELLS];
	u32 size_cells[MAX_SIZE_CELLS];
};

struct dt_bus {
	const char *name;
	int (*match)(const struct dt_bus *bus, int nodeoffset);
	/*
	 * match() returns
	 *  - a positive value on match
	 *  - zero on no match
	 *  - a negative value on error
	 */
	int (*translate)(const struct dt_bus *bus, struct dt_reg *reg,
			  void **addr, size_t *size);
	/*
	 * translate() returns
	 *  - zero on success
	 *  - a negative value on error
	 */
	void *private;
};

extern const struct dt_bus dt_default_bus;
extern void dt_bus_init_defaults(struct dt_bus *bus, const char *name);
extern int dt_bus_default_match(const struct dt_bus *bus, int nodeoffset);
extern int dt_bus_default_translate(const struct dt_bus *bus,
				    struct dt_reg *reg, void **addr,
				    size_t *size);

/*
 * find an fdt device node compatible with @compatible using match()
 * from the given bus @bus.
 */
extern int dt_bus_find_device_compatible(const struct dt_bus *bus,
					 const char *compatible);

/*
 * translate the reg indexed by @regidx of the "reg" property of the
 * device node at @nodeoffset using translate() from the given bus @bus.
 * returns the translation in @addr and @size
 */
extern int dt_bus_translate_reg(int nodeoffset, const struct dt_bus *bus,
				int regidx, void **addr, size_t *size);

/*
 * same as dt_bus_translate_reg, but uses the given @address_cells and
 * @size_cells rather than pulling them from the parent of @nodeoffset
 */
extern int __dt_bus_translate_reg(int nodeoffset, const struct dt_bus *bus,
				  int regidx, u32 *address_cells,
				  u32 *size_cells, void **addr,
				  size_t *size);

/*
 * read the "reg" property of @nodeoffset, which is defined by @address_cells
 * and @size_cells, and store the reg indexed by @regidx into @reg
 */
extern int dt_get_reg(int nodeoffset, int regidx, u32 *address_cells,
		      u32 *size_cells, struct dt_reg *reg);

/*
 * searches up the devicetree for @address_cells and @size_cells,
 * starting from @nodeoffset
 */
extern int dt_find_num_cells(int nodeoffset, u32 *address_cells,
			    u32 *size_cells);

/*
 * convert devicetree errors to strings
 */
extern const char *dt_strerror(int errval);

#endif
