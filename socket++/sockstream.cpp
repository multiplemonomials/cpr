// sockstream.C -*- C++ -*- socket library
// Copyright (C) 2002 Herbert Straub for my changes, see ChangeLog.
//
// Copyright (C) 1992-1996 Gnanasekaran Swaminathan <gs4t@virginia.edu>
//
// Permission is granted to use at your own risk and distribute this software
// in source and  binary forms provided  the above copyright notice and  this
// paragraph are  preserved on all copies.  This software is provided "as is"
// with no express or implied warranty.
//
// Version: 12Jan97 1.11
//
// You can simultaneously read and write into
// a sockbuf just like you can listen and talk
// through a telephone. Hence, the read and the
// write buffers are different. That is, they do not
// share the same memory.
// 
// Read:
// gptr() points to the start of the get area.
// The unread chars are gptr() - egptr().
// base() points to the read buffer
// 
// eback() is set to base() so that pbackfail()
// is called only when there is no place to
// putback a char. And pbackfail() always returns EOF.
// 
// Write:
// pptr() points to the start of the put area
// The unflushed chars are pbase() - pptr()
// pbase() points to the write buffer.
// epptr() points to the end of the write buffer.
// 
// Output is flushed whenever one of the following conditions
// holds:
// (1) pptr() == epptr()
// (2) EOF is written
// (3) linebuffered and '\n' is written
// 
// Unbuffered:
// Input buffer size is assumed to be of size 1 and output
// buffer is of size 0. That is, egptr() <= base()+1 and
// epptr() == pbase().
//
// Version: 1.2 2002-07-25 Herbert Straub 
// 	Improved Error Handling - extending the err class by cOperation

#include "sockstream.h"
#ifdef HAVE_SSTREAM
#include <sstream>
#else
#include <strstream>
#endif
#include <string>

#ifndef BUFSIZ
#  define BUFSIZ 1024
#endif

namespace socketpp
{

void sockbuf::initWSA()
{
#ifdef USE_WINSOCK
	if(!wsaStartupInfo)
	{
		wsaStartupInfo = std::make_shared<WSADATA>();
		int result = WSAStartup(MAKEWORD(2, 0), wsaStartupInfo.get());
		if(result != 0)
		{
			throw err(result, "sockinetbuf::initWSA()", "initializing WSA library");
		}
	}
#endif
}

#ifdef USE_WINSOCK
std::shared_ptr<WSADATA> sockbuf::wsaStartupInfo{};
#endif

sockbuf::sockbuf (const sockbuf::sockdesc& sd)
{
  rep = std::make_shared<sockbuf::sockcnt>(sd.sock);
  char_type* gbuf = new char_type [BUFSIZ];
  char_type* pbuf = new char_type [BUFSIZ];
  setg (gbuf, gbuf + BUFSIZ, gbuf + BUFSIZ);
  setp (pbuf, pbuf + BUFSIZ);
  rep->gend = gbuf + BUFSIZ;
  rep->pend = pbuf + BUFSIZ;
}	   

sockbuf::sockbuf(int domain, sockbuf::type st, int proto)
{
  initWSA();

  SOCKET soc = ::socket(domain, st, proto);
  
  if (soc == INVALID_SOCKET)
    throw err(lastError(), "sockbuf::sockbuf", "socket creation failed");

  rep = std::make_shared<sockbuf::sockcnt>(soc);;
  
  char_type* gbuf = new char_type [BUFSIZ];
  char_type* pbuf = new char_type [BUFSIZ];
  setg (gbuf, gbuf + BUFSIZ, gbuf + BUFSIZ);
  setp (pbuf, pbuf + BUFSIZ);
  rep->gend = gbuf + BUFSIZ;
  rep->pend = pbuf + BUFSIZ;
}

sockbuf::sockbuf (const sockbuf& sb)
:
//streambuf (sb),
rep (sb.rep)
{
  // the streambuf::streambuf (const streambuf&) is assumed
  // to haved handled pbase () and gbase () correctly.
}


sockbuf::~sockbuf ()
{
  overflow(eofValue); // flush write buffer
  
  //if the last instance is being destroyed, close socket and destroy object
  if (rep.use_count() == 1)
  {
    delete [] pbase ();
    delete [] eback ();
#ifndef WIN32
    	int c = close (rep->sock);
#else
		int c = closesocket(rep->sock);
#endif
    if (c == SOCKET_ERROR) 
			throw err(lastError(), "sockbuf::~sockbuf", sockname);
  }
}

int sockbuf::sync ()
// we never return -1 because we throw socketpp::err
// exception in the event of an error.
{
  if (pptr () && pbase () < pptr () && pptr () <= epptr ()) {
    // we have some data to flush
    try {
      write (pbase (), pptr () - pbase ());
    }
    catch (int wlen) {
      // write was not completely successful
#ifdef HAVE_SSTREAM
      std::stringstream sb;
#else
      std::strstream sb;
#endif
      std::string errMessage;
      errMessage += "(" + sockname + ")";
      if (wlen) 
			{
				// reposition unwritten chars
				char* pto = pbase ();
				char* pfrom = pbase () + wlen;
				int len = pptr () - pbase () - wlen;
				while (pfrom < pptr ()) *pto++ = *pfrom++;
				setp (pbase (), (char_type*) rep->pend);
				pbump (len);
				sb << " wlen=(" << wlen << ")";
				errMessage += sb.rdbuf()->str();
      }
      throw err(lastError(), "sockbuf::sync", errMessage);
    }

    setp (pbase (), (char_type*) rep->pend);
  }

  // we cannot restore input data back to the socket stream
  // thus we do not do anything on the input stream

  return 0;
}

/// return the number of chars in the input sequence
int sockbuf::showmanyc () const
{
  if (gptr () && gptr () < egptr ())
    return egptr () - gptr ();
  return 0;
}

sockbuf::int_type sockbuf::underflow ()
{
  if (gptr () == 0)
    return eofValue; // input stream has been disabled

  if (gptr () < egptr ())
    return (unsigned char) *gptr (); // eofValue is a -ve number; make it
                                     // unsigned to be diff from EOF

  int rlen = read (eback (), (char*) rep->gend - (char*) eback ());

  if (rlen == 0)
    return eofValue;

  setg (eback (), eback (), eback () + rlen);
  return (unsigned char) *gptr ();
}

sockbuf::int_type sockbuf::uflow ()
{
  int_type ret = underflow ();
  if (ret == eofValue)
    return eofValue;

  gbump (1);
  return ret;
}

std::streamsize sockbuf::xsgetn(char_type* s, std::streamsize n)
{
  int rval = showmanyc ();
  if (rval >= n) {
    memcpy (s, gptr (), n * sizeof (char_type));
    gbump (n);
    return n;
  }

  memcpy (s, gptr (), rval * sizeof (char_type));
  gbump (rval);

  if (underflow () != eofValue)
    return rval + xsgetn (s + rval, n - rval);
  
  return rval;
}

sockbuf::int_type sockbuf::pbackfail (int c)
{
  return eofValue;
}

/** if pbase () == 0, no write is allowed and thus return eofValue.
* if c == eofValue, we sync the output and return 0.
* if pptr () == epptr (), buffer is full and thus sync the output,
* insert c into buffer, and return c.
* In all cases, if error happens, throw exception. */
sockbuf::int_type sockbuf::overflow (sockbuf::int_type c)
{
  if (pbase () == 0)
    return eofValue;

  if (c == eofValue)
    return sync ();

  if (pptr () == epptr ())
    sync ();
  *pptr () = (char_type)c;
  pbump (1);
  return c;
}

std::streamsize sockbuf::xsputn(const char_type* s, std::streamsize n)
{
  int wval = epptr () - pptr ();
  if (n <= wval) {
    memcpy (pptr (), s, n * sizeof (char_type));
    pbump (n);
    return n;
  }

  memcpy (pptr (), s, wval * sizeof (char_type));
  pbump (wval);
  
  if (overflow () != eofValue)
    return wval + xsputn (s + wval, n - wval);

  return wval;
}

void sockbuf::bind (sockAddr& sa)
{
  if (::bind (rep->sock, sa.addr (), sa.size ()) == -1)
    throw err (errno, "sockbuf::bind", sockname);
}

void sockbuf::connect(sockAddr& sa)
{
  if (::connect(rep->sock, sa.addr (), sa.size()) == -1)
    throw err (errno, "sockbuf::connect", sockname);
}

void sockbuf::listen (int num)
{
  if (::listen (rep->sock, num) == -1)
    throw err (errno, "sockbuf::listen", sockname);
}

int sockbuf::read (void* buf, int len)
{
	#ifdef USE_WINSOCK
		//only recv exists on Windows
		return recv(buf, len);
	#else
		if (rep->rtmo != -1 && is_readready (rep->rtmo)==0) {
			throw err (ETIMEDOUT, "sockbuf::read", sockname);
		}
		
		if (rep->oob && atmark ())
			throw sockoob ();

		int rval = 0;
		if ((rval = ::read (rep->sock, (char*) buf, len)) == -1)
			throw err (errno, "sockbuf::read", sockname);
		return rval;
	#endif
}

/**
* Accept an incoming connection on this socket (note that the acceptor socket must be in listening mode).
* Returns a description of a new socket for that connection.
*/
sockbuf::sockdesc sockbuf::accept()
{
  int socfd = -1;
  
  socfd = ::accept(rep->sock, nullptr, nullptr);
  
  if(socfd == -1)
  {
		if(errno == EINVAL)
		{
			throw err(errno, "sockbuf::accept", "did you forget to call listen()?");
		}
		
		else
		{
			throw err(errno, "sockbuf::accept", sockname);
		}
	}
	
  return sockdesc(socfd);
}

int sockbuf::recv (void* buf, int len, int msgf)
{
  if (rep->rtmo != -1 && is_readready (rep->rtmo)==0)
    throw err (ETIMEDOUT, "sockbuf::recv", sockname);
  
  if (rep->oob && atmark ())
    throw sockoob ();

  int rval = 0;
  if ((rval = ::recv (rep->sock, (char*) buf, len, msgf)) == -1)
    throw err (errno, "sockbuf::recv", sockname);
  return rval;
}

int sockbuf::recvfrom (sockAddr& sa, void* buf, int len, int msgf)
{
  if (rep->rtmo != -1 && is_readready (rep->rtmo)==0)
    throw err (ETIMEDOUT, "sockbuf::recvfrom", sockname);
  
  if (rep->oob && atmark ())
    throw sockoob ();

  int rval = 0;
  int sa_len = sa.size ();
  
  if ((rval = ::recvfrom (rep->sock, (char*) buf, len,
                          msgf, sa.addr (), (socklen_t*) // LN
                          &sa_len)) == -1)
    throw err (errno, "sockbuf::recvfrom", sockname);
  return rval;
}

int sockbuf::write(const void* buf, int len)
/// upon error, write throws the number of bytes writen so far instead
/// of a socketpp::err.
{
	#ifdef USE_WINSOCK
		//only send exists on Windows
		return send(buf, len);
	#else
		char *pbuf = (char*) buf;
		if (rep->stmo != -1 && is_writeready (rep->stmo)==0)
			throw err (ETIMEDOUT, "sockbuf::write", sockname);
		
		int wlen=0;
		while(len>0) {
			int	wval = ::write (rep->sock, pbuf+wlen, len);
			if (wval == -1) throw wlen;
			len -= wval;
			wlen += wval;
		}
		return wlen; // == len if every thing is all right
	#endif
}

int sockbuf::send (const void* buf, int len, int msgf)
{
  char *pbuf = (char *) buf;
  if (rep->stmo != -1 && is_writeready (rep->stmo)==0)
    throw err (ETIMEDOUT, "sockbuf::send", sockname);
  
  int wlen=0;
  while(len>0) {
    int	wval = ::send (rep->sock, pbuf+wlen, len, msgf);
    if (wval == -1) throw wlen;
    len -= wval;
    wlen += wval;
  }
  return wlen;
}

int sockbuf::sendto (sockAddr& sa, const void* buf, int len, int msgf)
{
  char *pbuf = (char *) buf;
  if (rep->stmo != -1 && is_writeready (rep->stmo)==0)
    throw err (ETIMEDOUT, "sockbuf::sendto", sockname);
  
  int wlen=0;
  while(len>0) {
    int	wval = ::sendto (rep->sock, pbuf+wlen, len, msgf,
			 sa.addr (), sa.size());
    if (wval == -1) throw wlen;
    len -= wval;
    wlen += wval;
  }
  return wlen;
}

#ifdef	HAVE_SENDMSG

int sockbuf::recvmsg (msghdr* msg, int msgf)
{
  if (rep->rtmo != -1 && is_readready (rep->rtmo)==0)
    throw err (ETIMEDOUT, "sockbuf::recvmsg", sockname);
  
  if (rep->oob && atmark ())
    throw sockoob ();

  int rval = ::recvmsg(rep->sock, msg, msgf);
  if (rval == -1) throw err (errno, "sockbuf::recvmsg", sockname);
  return rval;
}

int sockbuf::sendmsg (msghdr* msg, int msgf)
{
  if (rep->stmo != -1 && is_writeready (rep->stmo)==0)
    throw err (ETIMEDOUT, "sockbuf::sendmsg", sockname);
  
  int wlen = ::sendmsg (rep->sock, msg, msgf);
  if (wlen == -1) throw 0;
  return wlen;
}

#endif // HAVE_SENDMSG

int sockbuf::sendtimeout (int wp)
{
  int oldstmo = rep->stmo;
  rep->stmo = (wp < 0) ? -1: wp;
  return oldstmo;
}

int sockbuf::recvtimeout (int wp)
{
  int oldrtmo = rep->rtmo;
  rep->rtmo = (wp < 0) ? -1: wp;
  return oldrtmo;
}

int sockbuf::is_readready (int wp_sec, int wp_usec) const
{
  fd_set fds;
  FD_ZERO (&fds);
  FD_SET (rep->sock, &fds);
  
  timeval tv;
  tv.tv_sec  = wp_sec;
  tv.tv_usec = wp_usec;
  
  int ret = select (rep->sock+1, &fds, 0, 0, (wp_sec == -1) ? 0: &tv);
  if (ret == -1) throw err (errno, "sockbuf::is_readready", sockname);
  return ret;
}

int sockbuf::is_writeready (int wp_sec, int wp_usec) const
{
  fd_set fds;
  FD_ZERO (&fds);
  FD_SET (rep->sock, &fds);
  
  timeval tv;
  tv.tv_sec  = wp_sec;
  tv.tv_usec = wp_usec;
  
  int ret = select (rep->sock+1, 0, &fds, 0, (wp_sec == -1) ? 0: &tv);
  if (ret == -1) throw err (errno, "sockbuf::is_writeready", sockname);
  return ret;
}

int sockbuf::is_exceptionpending (int wp_sec, int wp_usec) const
{
  fd_set fds;
  FD_ZERO (&fds);
  FD_SET  (rep->sock, &fds);
  
  timeval tv;
  tv.tv_sec = wp_sec;
  tv.tv_usec = wp_usec;
  
  int ret = select (rep->sock+1, 0, 0, &fds, (wp_sec == -1) ? 0: &tv);
  if (ret == -1) throw err (errno, "sockbuf::is_exceptionpending", sockname);
  return ret;
}

void sockbuf::shutdown (shuthow sh)
{
  switch (sh) {
  case shut_read:
    delete [] eback ();
    setg (0, 0, 0);
    break;
  case shut_write:
    delete [] pbase ();
    setp (0, 0);
    break;
  case shut_readwrite:
    shutdown (shut_read);
    shutdown (shut_write);
    break;
  }
  if (::shutdown(rep->sock, sh) == -1) throw err (errno, "sockbuf::shutdown", sockname);
}

int sockbuf::getopt (int op, void* buf, int len, int level) const
{
    if (::getsockopt (rep->sock, level, op, (char*) buf, (socklen_t*) // LN
                      &len) == -1)
    throw err (errno, "sockbuf::getopt", sockname);
  return len;
}

void sockbuf::setopt (int op, void* buf, int len, int level) const
{
  if (::setsockopt (rep->sock, level, op, (char*) buf, len) == -1)
    throw err (errno, "sockbuf::setopt", sockname);
}

sockbuf::type sockbuf::gettype () const
{
  int ty=0;
  getopt (so_type, &ty, sizeof (ty));
  return sockbuf::type(ty);
}

int sockbuf::clearerror () const
{
  int err=0;
  getopt (so_error, &err, sizeof (err));
  return err;
}

bool sockbuf::debug () const
{
  int old = 0;
  getopt (so_debug, &old, sizeof (old));
  return old!=0;
}

bool sockbuf::debug (bool set) const
{
  int old=0;
  int opt = set;
  getopt (so_debug, &old, sizeof (old));
  setopt (so_debug, &opt, sizeof (opt));
  return old!=0;
}

bool sockbuf::reuseaddr () const
{
  int old = 0;
  getopt (so_reuseaddr, &old, sizeof (old));
  return old!=0;
}

bool sockbuf::reuseaddr (bool set) const
{
  int old=0;
  int opt = set;
  getopt (so_reuseaddr, &old, sizeof (old));
  setopt (so_reuseaddr, &opt, sizeof (opt));
  return old!=0;
}

bool sockbuf::keepalive () const
{
  int old = 0;
  getopt (so_keepalive, &old, sizeof (old));
  return old!=0;
}

bool sockbuf::keepalive (bool set) const
{
  int old=0;
  int opt = set;
  getopt (so_keepalive, &old, sizeof (old));
  setopt (so_keepalive, &opt, sizeof (opt));
  return old!=0;
}

bool sockbuf::dontroute () const
{
  int old = 0;
  getopt (so_dontroute, &old, sizeof (old));
  return old!=0;
}

bool sockbuf::dontroute (bool set) const
{
  int old = 0;
  int opt = set;
  getopt (so_dontroute, &old, sizeof (old));
  setopt (so_dontroute, &opt, sizeof (opt));
  return old!=0;
}

bool sockbuf::broadcast () const
{
  int old=0;
  getopt (so_broadcast, &old, sizeof (old));
  return old!=0;
}

bool sockbuf::broadcast (bool set) const
{
  int old = 0;
  int opt = set;
  getopt (so_broadcast, &old, sizeof (old));
  setopt (so_broadcast, &opt, sizeof (opt));
  return old!=0;
}

bool sockbuf::oobinline () const
{
  int old=0;
  getopt (so_oobinline, &old, sizeof (old));
  return old!=0;
}
    
bool sockbuf::oobinline (bool set) const
{
  int old = 0;
  int opt = set;
  getopt (so_oobinline, &old, sizeof (old));
  setopt (so_oobinline, &opt, sizeof (opt));
  return old!=0;
}

bool sockbuf::oob (bool b)
{
  bool old = rep->oob;
  rep->oob = b;
  return old;
}

sockbuf::socklinger sockbuf::linger () const
{
  sockbuf::socklinger old (0, 0);
  getopt (so_linger, &old, sizeof (old));
  return old;
}

sockbuf::socklinger sockbuf::linger (sockbuf::socklinger opt) const
{
  socklinger old (0, 0);
  getopt (so_linger, &old, sizeof (old));
  setopt (so_linger, &opt, sizeof (opt));
  return old;
}

int sockbuf::sendbufsz () const
{
  int old=0;
  getopt (so_sndbuf, &old, sizeof (old));
  return old;
}

int sockbuf::sendbufsz (int sz) const
{
  int old=0;
  getopt (so_sndbuf, &old, sizeof (old));
  setopt (so_sndbuf, &sz, sizeof (sz));
  return old;
}

int sockbuf::recvbufsz () const
{
  int old=0;
  getopt (so_rcvbuf, &old, sizeof (old));
  return old;
}

int sockbuf::recvbufsz (int sz) const
{
  int old=0;
  getopt (so_rcvbuf, &old, sizeof (old));
  setopt (so_rcvbuf, &sz, sizeof (sz));
  return old;
}

bool sockbuf::atmark () const
/** return true, if the read pointer for socket points to an
* out of band data */
{
#ifndef WIN32
	int arg;
  if (::ioctl (rep->sock, SIOCATMARK, &arg) == -1)
    throw err (errno, "sockbuf::atmark", sockname);
#else
	unsigned long arg = 0;
  if (::ioctlsocket(rep->sock, SIOCATMARK, &arg) == SOCKET_ERROR)
    throw err (WSAGetLastError(), "sockbuf::atmark", sockname);
#endif // !WIN32
  return arg!=0;
}

#if !(defined(WIN32) || defined(__CYGWIN__))
/** return the process group id that would receive SIGIO and SIGURG
* signals */
int sockbuf::pgrp () const
{
  int arg;
  if (::ioctl (rep->sock, SIOCGPGRP, &arg) == -1)
    throw err (errno, "sockbuf::pgrp", sockname);
  return arg;
}

/** set the process group id that would receive SIGIO and SIGURG signals.
* return the old pgrp */
int sockbuf::pgrp (int new_pgrp) const
{
  int old = pgrp ();
  if (::ioctl (rep->sock, SIOCSPGRP, &new_pgrp) == -1)
    throw err (errno, "sockbuf::pgrp", sockname);
  return old;
}

/** if set is true, set close on exec flag
* else clear close on exec flag */
void sockbuf::closeonexec (bool set) const
{
  if (set) {
    if (::ioctl (rep->sock, FIOCLEX, 0) == -1)
      throw err (errno, "sockbuf::closeonexec", sockname);
  } else {
    if (::ioctl (rep->sock, FIONCLEX, 0) == -1)
      throw err (errno, "sockbuf::closeonexec", sockname);
  }
}

#endif

/** return how many chars are available for reading in the recvbuf of
* the socket. */
long sockbuf::nread () const
{
	long arg;
#ifndef WIN32  
  if (::ioctl (rep->sock, FIONREAD, &arg) == -1)
    throw err (errno, "sockbuf::nread", sockname);
#else
	if (::ioctlsocket (rep->sock, FIONREAD, (unsigned long *) &arg) == SOCKET_ERROR)
    throw err (WSAGetLastError(), "sockbuf::nread", sockname);
#endif // !WIN32
  return arg;
}

/** return how many chars are available for reading in the input buffer
* and the recvbuf of the socket. */
long sockbuf::howmanyc () const
{
  return showmanyc () + nread ();
}

/** if set is true, set socket to non-blocking io. Henceforth, any
* write or read operation will not wait if write or read would block.
* The read or write operation will result throwing a err
* exception with errno set to  EWOULDBLOCK. */
void sockbuf::nonblock (bool set) const
{
#ifndef WIN32
  int arg = set;
  if (::ioctl (rep->sock, FIONBIO, &arg) == -1)
    throw err (errno, "sockbuf::nonblock", sockname);
#else
  unsigned long arg = (set)?1:0;
  if (::ioctlsocket (rep->sock, FIONBIO, &arg) == -1)
    throw err (WSAGetLastError(), "sockbuf::nonblock", sockname);
#endif // !WIN32
}

/** if set is true, set socket for asynchronous io. If any io is
 possible on the socket, the process will get SIGIO */
#ifndef WIN32
void sockbuf::async (bool set) const
{
  int arg = set;
  if (::ioctl (rep->sock, FIOASYNC, &arg) == -1)
    throw err (errno, "sockbuf::async", sockname);
}
#endif // !WIN32

std::ostream& crlf(std::ostream& o)
{
  o << "\r\n";
  o.rdbuf()->pubsync();
  return o;
}

std::ostream& lfcr(std::ostream& o)
{
  o << "\n\r";
  o.rdbuf()->pubsync();
  return o;
}

}
