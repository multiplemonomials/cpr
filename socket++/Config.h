#define SOCKETPP_VERSION_MAJOR 2
#define SOCKETPP_VERSION_MINOR 0

/* #undef HAVE_FORK */

/* #undef HAVE_VFORK */

/* #undef HAVE_SYS_SIGLIST */

/* #undef HAVE_SENDMSG */

/* #undef HAVE_SIGACTION */

/* #undef HAVE_SELECT */

/* Define to 1 if you have the <dlfcn.h> header file. */
/* #undef HAVE_DLFCN_H */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <sstream> header file. */
#define HAVE_SSTREAM 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <string> header file. */
#define HAVE_STRING 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have <sys/wait.h> that is POSIX.1 compatible. */
/* #undef HAVE_SYS_WAIT_H */

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

//if there isn't a working vfork, make our own
#ifndef HAVE_VFORK
	#ifdef HAVE_FORK
		#define fork vfork
	#endif
#endif

#ifndef HAVE_SYS_SIGLIST
	#define _sys_siglist sys_siglist
#endif

#define EXTERN_C_BEGIN 
#define EXTERN_C_END 
