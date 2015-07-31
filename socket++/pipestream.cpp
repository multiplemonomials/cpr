#include "pipestream.h"

namespace socketpp
{

pipestreambuf::pipestreambuf(filelikebuf::filedesc fileDesc)
: filelikebuf(fileDesc)
{
	inherit(false);
}

pipepair createpipe()
{
#ifdef USE_WINSOCK
	HANDLE readPipe = 0;
	HANDLE writePipe = 0;

	SECURITY_ATTRIBUTES securityAttrs;
	memset(&securityAttrs, 0, sizeof(securityAttrs));
	securityAttrs.bInheritHandle = true;
	securityAttrs.nLength = sizeof(securityAttrs);
	securityAttrs.lpSecurityDescriptor = nullptr;

	if(!CreatePipe(&readPipe, &writePipe, &securityAttrs, 0))
	{

	}

#else
#endif
}

}
