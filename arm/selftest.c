#include "libcflat.h"
#include "arm/sysinfo.h"
#include "arm/ptrace.h"
#include "arm/processor.h"
#include "arm/asm-offsets.h"

#define PASS 0
#define FAIL 1

static struct pt_regs expected_regs;
/*
 * Capture the current register state and execute an instruction
 * that causes an exception. The test handler will check that its
 * capture of the current register state matches the capture done
 * here.
 *
 * NOTE: update clobber list if passed insns needs more than r0,r1
 */
#define test_exception(pre_insns, excptn_insn, post_insns)	\
	asm volatile(						\
		pre_insns "\n"					\
		"mov	r0, %0\n"				\
		"stmia	r0, { r0-lr }\n"			\
		"mrs	r1, cpsr\n"				\
		"str	r1, [r0, #" __stringify(S_PSR) "]\n"	\
		"mov	r1, #-1\n"				\
		"str	r1, [r0, #" __stringify(S_OLD_R0) "]\n"	\
		"add	r1, pc, #8\n"				\
		"str	r1, [r0, #" __stringify(S_R1) "]\n"	\
		"str	r1, [r0, #" __stringify(S_PC) "]\n"	\
		excptn_insn "\n"				\
		post_insns "\n"					\
	:: "r" (&expected_regs) : "r0", "r1")

static bool check_regs(struct pt_regs *regs)
{
	unsigned i;

	/* exception handlers should always run in svc mode */
	if (current_mode() != SVC_MODE)
		return false;

	for (i = 0; i < ARRAY_SIZE(regs->uregs); ++i) {
		if (regs->uregs[i] != expected_regs.uregs[i])
			return false;
	}

	return true;
}

static bool und_works;
static void und_handler(struct pt_regs *regs)
{
	und_works = check_regs(regs);
}

static bool check_und(void)
{
	install_exception_handler(EXCPTN_UND, und_handler);

	/* issue an instruction to a coprocessor we don't have */
	test_exception("", "mcr p2, 0, r0, c0, c0", "");

	install_exception_handler(EXCPTN_UND, NULL);

	return und_works;
}

static bool svc_works;
static void svc_handler(struct pt_regs *regs)
{
	u32 svc = *(u32 *)(regs->ARM_pc - 4) & 0xffffff;

	if (processor_mode(regs) == SVC_MODE) {
		/*
		 * When issuing an svc from supervisor mode lr_svc will
		 * get corrupted. So before issuing the svc, callers must
		 * always push it on the stack. We pushed it to offset 4.
		 */
		regs->ARM_lr = *(unsigned long *)(regs->ARM_sp + 4);
	}

	svc_works = check_regs(regs) && svc == 123;
}

static bool check_svc(void)
{
	install_exception_handler(EXCPTN_SVC, svc_handler);

	if (current_mode() == SVC_MODE) {
		/*
		 * An svc from supervisor mode will corrupt lr_svc and
		 * spsr_svc. We need to save/restore them separately.
		 */
		test_exception(
			"mrs	r0, spsr\n"
			"push	{ r0,lr }\n",
			"svc	#123\n",
			"pop	{ r0,lr }\n"
			"msr	spsr_cxsf, r0\n"
		);
	} else {
		test_exception("", "svc #123", "");
	}

	install_exception_handler(EXCPTN_SVC, NULL);

	return svc_works;
}

static void check_vectors(void)
{
	int ret = check_und() && check_svc() ? PASS : FAIL;
	exit(ret);
}

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

	} else if (strcmp(argv[0], "vectors") == 0) {

		check_vectors(); /* doesn't return */

	} else if (strcmp(argv[0], "vectors_usr") == 0) {

		start_usr(check_vectors); /* doesn't return */

	}

	return ret;
}
