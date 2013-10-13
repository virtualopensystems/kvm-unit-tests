#ifndef _ERRNO_H_
#define _ERRNO_H_
/*
 * Define some error codes for the test framework's use. qemu
 * exits with ((code << 1) | 1) when this framework calls
 * exit(code), so we reserve codes 64 to 126. 127 is left
 * for exit(-1).
 *
 * (Ab)use the standard E* names for syntax highlighting...
 * The errno descriptions in [] are for non-standard semantics.
 */
#define EINTR	(64 + 4)	/* [unhandled exception] */
#define EIO	(64 + 5)	/* I/O error */
#define ENXIO	(64 + 6)	/* No such device or address [no serial] */
#define ENOEXEC	(64 + 8)	/* Exec format error [bad flat file] */
#define ENOMEM	(64 + 12)	/* Out of memory */
#define ENODEV	(64 + 19)	/* No such device */
#define EINVAL	(64 + 22)	/* Invalid argument */
#define ENOSPC	(64 + 28)	/* No space left on device */
#define ERANGE	(64 + 34)	/* Math result not representable
				   [divide by zero] */
#endif
