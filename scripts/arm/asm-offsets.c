/*
 * Adapted from arch/arm/kernel/asm-offsets.c
 */
#include "libcflat.h"
#include "arm/ptrace.h"

#define P(sym, val) \
	printf("#define " #sym "\t0x%x\n", val)

int main(void)
{
	printf("#ifndef _ARM_ASM_OFFSETS_H_\n");
	printf("#define _ARM_ASM_OFFSETS_H_\n");
	printf("/*\n");
	printf(" * Generated file. Regenerate with 'make asm-offsets'\n");
	printf(" */\n");
	printf("\n");
	P(S_R0, offsetof(struct pt_regs, ARM_r0));
	P(S_R1, offsetof(struct pt_regs, ARM_r1));
	P(S_R2, offsetof(struct pt_regs, ARM_r2));
	P(S_R3, offsetof(struct pt_regs, ARM_r3));
	P(S_R4, offsetof(struct pt_regs, ARM_r4));
	P(S_R5, offsetof(struct pt_regs, ARM_r5));
	P(S_R6, offsetof(struct pt_regs, ARM_r6));
	P(S_R7, offsetof(struct pt_regs, ARM_r7));
	P(S_R8, offsetof(struct pt_regs, ARM_r8));
	P(S_R9, offsetof(struct pt_regs, ARM_r9));
	P(S_R10, offsetof(struct pt_regs, ARM_r10));
	P(S_FP, offsetof(struct pt_regs, ARM_fp));
	P(S_IP, offsetof(struct pt_regs, ARM_ip));
	P(S_SP, offsetof(struct pt_regs, ARM_sp));
	P(S_LR, offsetof(struct pt_regs, ARM_lr));
	P(S_PC, offsetof(struct pt_regs, ARM_pc));
	P(S_PSR, offsetof(struct pt_regs, ARM_cpsr));
	P(S_OLD_R0, offsetof(struct pt_regs, ARM_ORIG_r0));
	P(S_FRAME_SIZE, sizeof(struct pt_regs));
	printf("\n");
	printf("#endif /* _ARM_ASM_OFFSETS_H_ */\n");
	return 0;
}
