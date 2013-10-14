#include "libcflat.h"
#include "arm/sysinfo.h"

#define PASS 0
#define FAIL 1

static void assert_enough_args(int nargs, int needed)
{
	if (nargs < needed) {
		printf("Not enough arguments.\n");
		exit(EINVAL);
	}
}

int main(int argc, char **argv)
{
	int ret = FAIL;

	assert_enough_args(argc, 1);

	if (strcmp(argv[0], "mem") == 0) {

		assert_enough_args(argc, 2);

		if (mem_size/1024/1024 == (size_t)atol(argv[1]))
			ret = PASS;
	}

	return ret;
}
