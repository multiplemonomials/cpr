#define CPR_VERSION_MAJOR 1
#define CPR_VERSION_MINOR 0

#define HAVE_FORK 1

#define HAVE_VFORK 1

#define HAVE_SYS_SIGLIST 1

#define HAVE_SENDMSG 1

#define HAVE_SIGACTION 1

/* Define to 1 if you have the `select' function. */
#define HAVE_SELECT 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

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
#define HAVE_SYS_WAIT_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

//if there isn't a working vfork, make our own
#ifndef HAVE_VFORK
	#define fork vfork
#endif

#ifndef HAVE_SYS_SIGLIST
	#define _sys_siglist sys_siglist
#endif

/* Define if extern "C" is needed arround include files */
#define EXTERN_C_BEGIN /**/
#define EXTERN_C_END /**/
