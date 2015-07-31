// pipestream.cpp  -*- C++ -*- socket library
// Copyright (C) 2002 Herbert Straub
//
// pipestream.C -*- C++ -*- socket library
// Copyright (C) 1992-1996 Gnanasekaran Swaminathan <gs4t@virginia.edu>
//
// Permission is granted to use at your own risk and distribute this software
// in source and  binary forms provided  the above copyright notice and  this
// paragraph are  preserved on all copies.  This software is provided "as is"
// with no express or implied warranty.
//
// Version: 12Jan97 1.11
// 2002-07-28 Version 1.2 (C) Herbert Straub
//  Eliminating sorry_about_global_temp inititialisation. This don't work
//  in combination with NewsCache. My idea is: initializing the classes with (0)
//  and in the second step call std::ios::init (sockbuf *) and std::iosockstream::init ...
//  The constructors of ipipestream, opipestream and iopipestream are changed.

#include "ossock.h"
#include "processstream.h"

#include <utility>

namespace socketpp
{

#ifdef USE_WINSOCK

pipepair createprocessstream(const char* cmd)
{
	//see https://msdn.microsoft.com/en-us/library/windows/desktop/ms682499%28v=vs.85%29.aspx
	pipepair stdoutpipe = createpipe();
	pipepair stdinpipe = createpipe();

	//stop the ends we want to keep from being inherited
	stdoutpipe.second->inherit(false);
	stdinpipe.first->inherit(false);

	STARTUPINFO startupInfo;
	memset(&startupInfo, 0, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);
	startupInfo.hStdError = stdoutpipe.first->fd();
	startupInfo.hStdOutput = stdoutpipe.first->fd();
	startupInfo.hStdInput = stdinpipe.second->fd();
	startupInfo.dwFlags = STARTF_USESTDHANDLES;

	PROCESS_INFORMATION procInfo;
	memset(&procInfo, 0, sizeof(procInfo));

	std::string args = " /c " + std::string(cmd);

	int retval = CreateProcess("cmd.exe", const_cast<LPSTR>(args.c_str()), nullptr, nullptr, true, 0, nullptr, nullptr, &startupInfo, &procInfo);
	if(retval != 0)
	{
		throw err(lastError(), "createprocessstream", "while invoking CreateProcess");
	}

	//for some reason, just pipepair here doesn't work
	return std::pair<std::shared_ptr<ipipestream>, std::shared_ptr<opipestream>>(stdinpipe.first, stdoutpipe.second);
}

#else
// child closes s2 and uses s1
// parent closes s1 and uses s2
pipepair createprocessstream(const char* cmd)
{
	pipepair stdoutpipe = createpipe();
	pipepair stdinpipe = createpipe();

	pid_t pid = ::fork();
	if (pid == -1) throw err(errno, "socketpp::createprocessstream()", "while invoking fork");

	if (pid == 0) {

		//redirect stdout and stdin to the first socket
		if (::dup2(stdoutpipe.first->fd(), fileno(stdout)) == -1)
		  throw err(errno);
		if (::dup2(stdinpipe.second->fd(), fileno(stdin)) == -1)
		  throw err(errno);

		const char*	argv[4];
		argv[0] = "/bin/sh";
		argv[1] = "-c";
		argv[2] = cmd;
		argv[3] = '\0';
		execv("/bin/sh", (char**) argv);

		//if we get to here, it's because execv failed (I think).
		//unfortunately, we've already rebound stdout, so there's no one to hear us scream.
		_exit(1);
	}

	// parent process
	return std::pair<std::shared_ptr<ipipestream>, std::shared_ptr<opipestream>>(stdinpipe.first, stdoutpipe.second);
}

#endif

}


