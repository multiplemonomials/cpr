/*

This header includes all of the headers for the OS's socket library, whichever one it is.

*/

#ifndef OSSOCK_H
#define OSSOCK_H

#include "socketpp-config.h"

//header common to both libraries
#include <sys/types.h>

#ifdef USE_WINSOCK
# include <Winsock2.h>
# include <Windows.h>
#	include <Wininet.h>
#	pragma comment(lib, "Wininet.lib")
#	pragma comment(lib, "Ws2_32.lib")
# include <Winerror.h>
#else
	EXTERN_C_BEGIN
#	include <sys/uio.h>
#	include <sys/socket.h>
#	include <sys/time.h>
#	include <sys/socket.h>
#	include <sys/ioctl.h>
#	include <unistd.h>
#	include <errno.h>
# include <sys/un.h>
#	include <netinet/tcp.h>
# include <arpa/inet.h>
# include <netinet/in.h>
#	include <netdb.h>
#	include <sys/param.h>
# include <fcntl.h>
#include <sys/wait.h>
	EXTERN_C_END
#endif


/*
	QUIRKS SECTION START
	-----------------------------------------------------------------------------------------------

	Random platform-based typedefs were littered across the code.  I've collected all of them here.
*/

//standardized function to get the most recent error code
#ifdef USE_WINSOCK
	#define lastError() WSAGetLastError()
#else
	#define lastError() errno
#endif

#if defined(__linux__) || defined(__CYGWIN__)
#  define MSG_MAXIOVLEN	 16
#endif // __linux__

#ifdef USE_WINSOCK
	# define	MAXPATHLEN MAX_PATH
#endif

#ifndef USE_WINSOCK
	#define SOCKET int
	#define INVALID_SOCKET -1
	#define SOCKET_ERROR -1
#endif

#if defined(__APPLE) || defined(USE_WINSOCK)
typedef int socklen_t;
#endif

#ifdef FD_ZERO
#  undef FD_ZERO    // bzero causes so much trouble for us
#endif
#define FD_ZERO(p) (memset ((p), 0, sizeof *(p)))

#if defined(USE_WINSOCK)
	//figure out what kind of error codes to use in socket::err
	#if defined(EWOULDBLOCK)
		#define USE_POSIX_MINGW_ERRORS  //enable error codes on linux and MinGW
	#endif

	#define USE_WINSOCK_ERRORS

#else
#define USE_POSIX_MINGW_ERRORS  //enable error codes on linux and MinGW
#define USE_POSIX_ONLY_ERRORS  //enable error codes specific to linux
#endif


#endif // OSSOCK_H
