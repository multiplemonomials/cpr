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

#ifndef WIN32

#include <Config.h>
#include <base/cprerr.h>
#include <base/pipestream.h>

#include <unistd.h>

// environ is not given a declaration in sun's <unistd.h>
extern char** environ;

namespace cpr
{

// child closes s2 and uses s1
// parent closes s1 and uses s2

enum domain { af_unix = 1 };

///Returns a pair of linked Unix sockets.vc
std::pair<socketpp::iosockunix, socketpp::iosockunix> make_socketpair()
{
	int sockets[2];
  if (::socketpair(af_unix, socketpp::sockbuf::sock_stream, 0, sockets) == -1)
    throw cpr::exception(errno, "cpr::make_socketpair", "while invoking socketpair");
   
  return std::pair<socketpp::iosockunix, socketpp::iosockunix>(socketpp::iosockunix(socketpp::sockbuf::sockdesc(sockets[0])),
  	socketpp::iosockunix(socketpp::sockbuf::sockdesc(sockets[1])));
}

socketpp::sockbuf* detail::createpipestream(const char* cmd, int mode)
{
  int sockets[2];
  if (::socketpair(af_unix, socketpp::sockbuf::sock_stream, 0, sockets) == -1)
    throw cpr::exception(errno, "cpr::pipestream()", "while invoking socketpair");
  
  pid_t pid = ::fork();
  if (pid == -1) throw cpr::exception(errno, "cpr::pipestream()", "while invoking fork");
  
  if (pid == 0) {
    // child process
    if (::close(sockets[1]) == -1) throw cpr::exception(errno);

		//redirect stdout and stdin to the first socket
    if ((mode & std::ios::in) && ::dup2(sockets[0], fileno(stdout)) == -1)
      throw cpr::exception(errno);
    if ((mode & std::ios::out) && ::dup2(sockets[0], fileno(stdin)) == -1)
      throw cpr::exception(errno);
      
    //close the first socket, although the OS will keep piping stuff through it
    if (::close(sockets[0]) == -1) throw cpr::exception(errno);

    const char*	argv[4];
    argv[0] = "/bin/sh";
    argv[1] = "-c";
    argv[2] = cmd;
    argv[3] = '\0';
    execve("/bin/sh", (char**) argv, environ);
    _exit(0);
  }

  // parent process
  if (::close(sockets[0]) == -1) throw cpr::exception (errno);

  socketpp::sockbuf* s = new socketpp::sockbuf(socketpp::sockbuf::sockdesc(sockets[1]));
  
  if (!(mode & std::ios::out)) s->shutdown(socketpp::sockbuf::shut_write);
  if (!(mode & std::ios::in)) s->shutdown(socketpp::sockbuf::shut_read);
  
  return s;
}

ipipestream::ipipestream (const char* cmd)
  : socketpp::sockbuf(*detail::createpipestream(cmd, std::ios::in)),
  std::istream(this)
{

}

opipestream::opipestream (const char* cmd)
  : socketpp::sockbuf(*detail::createpipestream(cmd, std::ios::out)),
  std::ostream(this)
{

}

iopipestream::iopipestream (const char* cmd)
  : socketpp::sockbuf(*detail::createpipestream(cmd, std::ios::in|std::ios::out)),
  std::iostream(this)
{

}

}

#else

#warning pipestream, sockunix, and fork are not supported in a win32 environment

#endif

