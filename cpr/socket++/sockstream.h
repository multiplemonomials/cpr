// sockstream.h -*- C++ -*- socket library
// Copyright (C) 2002 Herbert Straub
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
// Version: 1.2 2002-07-25 Herbert Straub 
// 	Improved Error Handling - extending the sockerr class by cOperation
// 2003-03-06 Herbert Straub
// 	adding sockbuf::getname und setname (sockname)
// 	sockbuf methods throw method name + sockname

#ifndef _SOCKSTREAM_H
#define	_SOCKSTREAM_H

#include <base/cprerr.h>

#include <iostream> // must be ANSI compatible
#include <cstddef>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <cstdio>
#include <memory>

#ifndef WIN32
#	include <sys/types.h>
#	include <sys/uio.h>
#	include <sys/socket.h>
#	define SOCKET int
#	define SOCKET_ERROR -1
#else
#	include <Wininet.h>
#	pragma comment(lib, "Wininet")
#endif

#if defined(__linux__) || defined(__CYGWIN__)
#  define MSG_MAXIOVLEN	 16
#endif // __linux__

// socket address classes
struct sockaddr;

struct msghdr;

namespace socketpp
{

class sockoob {
public:
  const char* what () const { return "sockoob"; }
};  
 
 //abstract base class for socket addresses
class sockAddr 
{
public:
  virtual		~sockAddr() {}
    
  virtual		operator void*	() const =0;
                        operator sockaddr* () const { return addr (); }
                        
  /**
  * Returns size in bytes of the implementation.
  * Equivalent to sizeof(someSockAddrImplementer)
  */
  virtual int		size 		() const =0;
  virtual int		family		() const =0;
  virtual sockaddr*     addr 		() const =0;
};

// socket buffer class
class sockbuf: public std::streambuf 
{
public:
  enum type 
  {
    sock_stream	        = SOCK_STREAM,
    sock_dgram	        = SOCK_DGRAM,
    sock_raw	        = SOCK_RAW,
    sock_rdm	        = SOCK_RDM,
    sock_seqpacket      = SOCK_SEQPACKET
  };
  enum option 
  {
    so_debug	        = SO_DEBUG,
    so_reuseaddr	= SO_REUSEADDR,
    so_keepalive	= SO_KEEPALIVE,
    so_dontroute	= SO_DONTROUTE,
    so_broadcast	= SO_BROADCAST,
    so_linger	        = SO_LINGER,
    so_oobinline	= SO_OOBINLINE,
    so_sndbuf		= SO_SNDBUF,
    so_rcvbuf		= SO_RCVBUF,
    so_error		= SO_ERROR,
    so_type		= SO_TYPE
  };
  
  enum level 
  {
    sol_socket          = SOL_SOCKET
  };
  
  enum msgflag 
  {
    msg_oob		= MSG_OOB,
    msg_peek	        = MSG_PEEK,
    msg_dontroute	= MSG_DONTROUTE,

#if !(defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__APPLE__))
    msg_maxiovlen	= MSG_MAXIOVLEN
#endif
  };
  
  enum shuthow 
  {
    shut_read,
    shut_write,
    shut_readwrite
  };
  
  enum 
  { 
  somaxconn	= SOMAXCONN
  };
  
  struct socklinger 
  {
    int	l_onoff;	// option on/off
    int	l_linger;	// linger time

    socklinger (int a, int b): l_onoff (a), l_linger (b) {}
  };

  typedef char          char_type;
  typedef std::streampos     pos_type;
  typedef std::streamoff     off_type;
  typedef int           int_type;
  typedef int           seekdir;
  
  enum 
  { 
	eofValue = EOF 
  }; // LN


  /**
  * Class that holds the OS handle of a socket
  */
  struct sockdesc 
  {
    int sock;
    sockdesc (int d): sock (d) {}
  };

protected:

  //internal impl object that stores the socket info
  //allows the outer object to be passed by value
  struct sockcnt 
  {
    SOCKET	sock;
    int			stmo; // -1==block, 0==poll, >0 == waiting time in secs
    int			rtmo; // -1==block, 0==poll, >0 == waiting time in secs
    bool		oob;	// check for out-of-band byte while reading
    void*		gend; // end of input buffer
    void*		pend; // end of output buffer

    sockcnt(SOCKET s)
      : sock(s), stmo(-1), rtmo(-1), oob(false),
        gend(0), pend(0) 
		{
		
		}
	
		sockcnt(sockcnt &) = delete;
  };

  std::shared_ptr<sockbuf::sockcnt> rep;  // pointer to sock
  std::string		sockname; // name of sockbuf - Herbert Straub

  virtual int           sync ();
  
  virtual int           showmanyc () const;
  virtual std::streamsize    xsgetn (char_type* s, std::streamsize n);
  virtual int_type      underflow ();
  virtual int_type      uflow ();

  virtual int_type      pbackfail (int_type c = eofValue);

  virtual std::streamsize    xsputn (const char_type* s, std::streamsize n);
  virtual int_type      overflow (int_type c = eofValue);

public:
  sockbuf (const sockdesc& sd);
  sockbuf (int domain, type, int proto);
  sockbuf (const sockbuf&);
  
  virtual 		~sockbuf ();

  int                   sd () const { return rep->sock; }
  int                   pubsync () { return sync (); }
    
  virtual void		bind	(sockAddr&);
  virtual void		connect	(sockAddr&);
    
  void		        listen	(int num=somaxconn);
  
	/**
	* Accept an incoming connection on this socket (note that the acceptor socket must be in listening mode)
	*
	* Template argument is the type of socket you want to construct, e.g. somesock.accept<socketpp::isockinet>()
	*/
	virtual sockdesc accept();
    
  int   		read	(void* buf, int len);
  int			recv	(void* buf, int len, int msgf=0);
  int			recvfrom(sockAddr& sa,
				 void* buf, int len, int msgf=0);

#ifdef HAVE_SENDMSG
  int			recvmsg (msghdr* msg, int msgf=0);
  int			sendmsg	(msghdr* msg, int msgf=0);
#endif
    
  int			write	(const void* buf, int len);
  int			send	(const void* buf, int len, int msgf=0);
  int			sendto	(sockAddr& sa,
				 const void* buf, int len, int msgf=0);
    
  int			sendtimeout (int wp=-1);
  int			recvtimeout (int wp=-1);
  int			is_readready (int wp_sec, int wp_usec=0) const;
  int			is_writeready (int wp_sec, int wp_usec=0) const;
  int			is_exceptionpending (int wp_sec, int wp_usec=0) const;
    
  void		        shutdown (shuthow sh);
    
  int			getopt(int op, void* buf, int len,
			       int level=sol_socket) const;
  void		        setopt(int op, void* buf, int len,
		               int level=sol_socket) const;
    
  type		        gettype () const;
  int			clearerror () const;
  bool			debug	  () const;
  bool			debug	  (bool set) const;
  bool			reuseaddr () const;
  bool			reuseaddr (bool set) const;
  bool			keepalive () const;
  bool			keepalive (bool set) const;
  bool			dontroute () const;
  bool			dontroute (bool set) const;
  bool			broadcast () const;
  bool			broadcast (bool set) const;
  bool			oobinline () const;
  bool			oobinline (bool set) const;
  bool                  oob       () const { return rep->oob; }
  bool                  oob       (bool b);
  int			sendbufsz () const;
  int			sendbufsz (int sz)   const;
  int			recvbufsz () const;
  int			recvbufsz (int sz)   const;
  socklinger            linger    () const;
  socklinger		linger    (socklinger opt) const;
  socklinger            linger    (int onoff, int tm) const
    { return linger (socklinger (onoff, tm)); }

  bool                  atmark    () const;  
  long                  nread     () const;
  long                  howmanyc  () const;
  void                  nonblock  (bool set=true) const;
  inline void		setname	  (const char *name);
  inline void		setname	  (const std::string &name);
  inline const std::string&	getname	  ();

#if !defined(WIN32)
  void                  async     (bool set=true) const;
	int                   pgrp      () const;
  int                   pgrp      (int new_pgrp) const;
  void                  closeonexec (bool set=true) const;
#endif
};

//holds functions common between every sockstream
//as well as the buffer
class sockstream_base : public virtual std::ios
{
  public:
	
  sockbuf*	rdbuf()
  {
		return (sockbuf*)rdbuf();
  }
  
  virtual ~sockstream_base()
  {

  }
};

//base class for socket input streams
class isockstream: public std::istream, public virtual sockstream_base
{
protected:
	isockstream();
	
public:
	isockstream(sockbuf* sb): sockstream_base(), std::istream(sb){}
	virtual ~isockstream () {}
};

//base class for socket output streams
class osockstream: public std::ostream, public virtual sockstream_base 
{
protected:
    osockstream();
	
public:
	osockstream(sockbuf* sb): sockstream_base(), std::ostream(sb) {}
	virtual ~osockstream () {}
};

//base class for socket iostreams
class iosockstream: public std::iostream, public virtual sockstream_base
{
protected:
    iosockstream();
	
public:
	iosockstream(sockbuf::sockdesc sockdesc):  sockstream_base(), std::iostream(new sockbuf(sockdesc)) {}
	
	iosockstream(const iosockstream&) = default;

    iosockstream(sockbuf* sb):  sockstream_base(), std::iostream(sb) {}
  virtual ~iosockstream () {}
};

// manipulators
extern std::ostream& crlf (std::ostream&);
extern std::ostream& lfcr (std::ostream&);

// inline

void sockbuf::setname (const char *name)
{
	sockname = name;
}
void sockbuf::setname (const std::string &name)
{
	sockname = name;
}
const std::string& sockbuf::getname ()
{
	return sockname;
}

}

#endif	// _SOCKSTREAM_H
