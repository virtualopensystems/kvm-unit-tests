
.arm

.section .init

.globl start
start:
	/*
	 * bootloader params are in r0-r2
	 * See the kernel doc Documentation/arm/Booting
	 */
	ldr	sp, =stacktop
	bl	setup

	/* start the test */
	ldr	r0, =__argc
	ldr	r0, [r0]
	ldr	r1, =__argv
	bl	main
	bl	exit
	b	halt

.text

.globl halt
halt:
1:	wfi
	b	1b
