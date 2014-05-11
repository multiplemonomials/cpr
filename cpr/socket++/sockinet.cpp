// sockinet.C  -*- C++ -*- socket library
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
// 2002-07-25 Version 1.2 (C) Herbert Straub
// 	Adding improved Error Handling in cpr::exception class
// 	sockinetaddr::setport if the first character of the port parameter is a 
// 		digit, then the parameter is interpreted as a number
// 2002-07-28 Version 1.2 (C) Herbert Straub
//  Eliminating sorry_about_global_temp inititialisation. This don't work
//  in combination with NewsCache. My idea is: initializing the classes with (0)
//  and in the second step call ios::init (sockinetbuf *) and iosockstream::init ...
//  The constructors of isockinet, osockinet and iosockinet are changed.

#include <Config.h>
#include <sockinet.h>
#if defined(__APPLE)
typedef int socklen_t;
#endif

#ifndef WIN32
	EXTERN_C_BEGIN
#	include <netdb.h>
#	include <sys/time.h>
#	include <sys/socket.h>
#	include <stdlib.h>
#	include <unistd.h>
#	include <errno.h>
#	include <netinet/tcp.h>
#  	include <arpa/inet.h>
#  	include <netinet/in.h>
	EXTERN_C_END
#else
#	define socklen_t int
# define EADDRNOTAVAIL				WSAEADDRNOTAVAIL
# define EADDRINUSE						WSAEADDRINUSE
#	define ENOPROTOOPT					WSAENOPROTOOPT
#endif // !WIN32


void	herror(const char*);

namespace socketpp
{

sockinetaddr::sockinetaddr () 
{
  sin_family	  = sockinetbuf::af_inet;
  sin_addr.s_addr = htonl(INADDR_ANY);
  sin_port	  = 0;
}

sockinetaddr::sockinetaddr(unsigned long addr, int port_no)
// addr and port_no are in host byte order
{
  sin_family      = sockinetbuf::af_inet;
  sin_addr.s_addr = htonl(addr);
  sin_port	  = htons(port_no);
}

sockinetaddr::sockinetaddr(unsigned long addr, const char* sn, const char* pn)
// addr is in host byte order
{
  sin_family      = sockinetbuf::af_inet;
  sin_addr.s_addr = htonl (addr); // Added by cgay@cs.uoregon.edu May 29, 1993
  setport(sn, pn);
}

sockinetaddr::sockinetaddr (const char* host_name, int port_no)
// port_no is in host byte order
{
  setaddr(host_name);
  sin_port = htons(port_no);
}

sockinetaddr::sockinetaddr(const char* hn, const char* sn, const char* pn)
{
  setaddr(hn);
  setport(sn, pn);
}

sockinetaddr::sockinetaddr (const sockinetaddr& sina)
{
  sin_family      = sockinetbuf::af_inet;
  sin_addr.s_addr = sina.sin_addr.s_addr;
  sin_port	  = sina.sin_port;
}   

void sockinetaddr::setport(const char* sn, const char* pn)
{
	if (isdigit (*sn)) {
		sin_port = htons(atoi(sn));
	} else {
  	servent* sp = getservbyname(sn, pn);
  	if (sp == 0) throw cpr::exception (EADDRNOTAVAIL, "sockinetaddr::setport");
  	sin_port = sp->s_port;
	}
}

int sockinetaddr::getport () const
{
  return ntohs (sin_port);
}

void sockinetaddr::setaddr(const char* host_name)
{
  if ( (sin_addr.s_addr = inet_addr(host_name)) == INADDR_NONE)
	 {
    hostent* hp = gethostbyname(host_name);

    if (hp == 0) throw cpr::exception (EADDRNOTAVAIL, "sockinetaddr::setaddr");

    memcpy(&sin_addr, hp->h_addr, hp->h_length);

    sin_family = hp->h_addrtype;
  }
	else
	{
    sin_family = sockinetbuf::af_inet;
	}
}

const char* sockinetaddr::gethostname () const
{
  if (sin_addr.s_addr == htonl(INADDR_ANY)) {
    static char hostname[64];
    if (::gethostname(hostname, 63) == -1) return "";
    return hostname;		
  }
  
  hostent* hp = gethostbyaddr((const char*) &sin_addr,
			      sizeof(sin_addr),
			      family());
  if (hp == 0) return "";
  if (hp->h_name) return hp->h_name;
  return "";
}

sockinetbuf::sockinetbuf (const sockbuf::sockdesc& sd)
  : sockbuf (sd.sock)
{}

sockinetbuf::sockinetbuf(sockbuf::type ty, int proto)
  : sockbuf (af_inet, ty, proto)
{}

sockinetaddr sockinetbuf::localaddr() const
{
  sockinetaddr sin;
  int len = sin.size();
  if (::getsockname(rep->sock, sin.addr (), (socklen_t*) // LN
                    &len) == -1)
    throw cpr::exception (errno, "sockinetbuf::localaddr");
  return sin;
}

int sockinetbuf::localport() const
{
  sockinetaddr sin = localaddr();
  if (sin.family() != af_inet) return -1;
  return sin.getport();
}

const char* sockinetbuf::localhost() const
{
  sockinetaddr sin = localaddr();
  if (sin.family() != af_inet) return "";
  return sin.gethostname();
}

sockinetaddr sockinetbuf::peeraddr() const
{
  sockinetaddr sin;
  int len = sin.size();
  if (::getpeername(rep->sock, sin.addr (), (socklen_t*) // LN
                    &len) == -1)
    throw cpr::exception (errno, "sockinetbuf::peeraddr");
  return sin;
}

///return the port of the remote, in host byte order
int sockinetbuf::peerport() const
{
  sockinetaddr sin = peeraddr();
  if (sin.family() != af_inet) return -1;
  return sin.getport();
}

const char* sockinetbuf::peerhost() const
{
  sockinetaddr sin = peeraddr();
  if (sin.family() != af_inet) return "";
  return sin.gethostname();
}

/** a. bind to (INADDR_ANY, portno)
* b. if success return the port number connected to
* c. if failure and errno is EADDRINUSE, portno++ and go to step a. */
int sockinetbuf::bind_until_success (int portno)
{
  for (;;) 
  {
    try 
	{
      bind(portno);
	  return portno;
    }
    catch (cpr::exception & e) 
	{
	  ++portno;
      if(e.get_errno () != EADDRINUSE) throw; // LN
      continue;
    }
    break;
  }
}

void sockinetbuf::bind (sockAddr& sa)
{
  sockbuf::bind (sa);
}

void sockinetbuf::bind (int port_no)
{
    sockinetaddr sa ((long unsigned int) // LN
                     INADDR_ANY, port_no);
  bind (sa);
}

void sockinetbuf::bind (unsigned long addr, int port_no)
// address and portno are in host byte order
{
  sockinetaddr sa (addr, port_no);
  bind (sa);
}

void sockinetbuf::bind (const char* host_name, int port_no)
{
  sockinetaddr sa (host_name, port_no);
  bind (sa);
}

void sockinetbuf::bind (unsigned long addr,
			const char* service_name,
			const char* protocol_name)
{
  sockinetaddr sa (addr, service_name, protocol_name);
  bind (sa);
}

void sockinetbuf::bind (const char* host_name,
			const char* service_name,
			const char* protocol_name)
{
  sockinetaddr sa (host_name, service_name, protocol_name);
  bind (sa);
}

void sockinetbuf::connect (sockAddr& sa)
{
  sockbuf::connect (sa);
}

void sockinetbuf::connect (unsigned long addr, int port_no)
// address and portno are in host byte order
{
  sockinetaddr sa (addr, port_no);
  connect (sa);
}

void sockinetbuf::connect (const char* host_name, int port_no)
{
  sockinetaddr sa (host_name, port_no);
  connect (sa);
}

void sockinetbuf::connect (unsigned long addr,
			   const char* service_name,
			   const char* protocol_name)
{
  sockinetaddr sa (addr, service_name, protocol_name);
  connect (sa);
}

void sockinetbuf::connect (const char* host_name,
			   const char* service_name,
			   const char* protocol_name)
{
  sockinetaddr sa (host_name, service_name, protocol_name);
  connect (sa);
}

bool sockinetbuf::tcpnodelay () const
{
  struct protoent* proto = getprotobyname ("tcp");
  if (proto == 0) throw cpr::exception (ENOPROTOOPT, "sockinetbuf::tcpnodelay");

  int old = 0;
  getopt (TCP_NODELAY, &old, sizeof (old), proto->p_proto);
  return old!=0;
}

bool sockinetbuf::tcpnodelay (bool set) const
{
  struct protoent* proto = getprotobyname ("tcp");
  if (proto == 0) throw cpr::exception (ENOPROTOOPT, "sockinetbuf::tcpnodelay");

  int old = 0;
  int opt = set;
  getopt (TCP_NODELAY, &old, sizeof (old), proto->p_proto);
  setopt (TCP_NODELAY, &opt, sizeof (opt), proto->p_proto);
  return old!=0;
}

isockinet::isockinet(const sockbuf::sockdesc& sd)
  : sockinet_base(sd), isockstream(this)
{

}

isockinet::isockinet(sockbuf::type ty, int proto)
  : sockinet_base(ty, proto), isockstream(this)
{

}

isockinet::isockinet(const sockinetbuf& sb)
  : sockinet_base(sb), isockstream(this)
{

}

isockinet::~isockinet ()
{
}

osockinet::osockinet(const sockbuf::sockdesc& sd)
  : sockinet_base(sd), osockstream(this)
{

}


osockinet::osockinet(sockbuf::type ty, int proto)
  : sockinet_base(ty, proto), osockstream(this)
{

}

osockinet::osockinet(const sockinetbuf& sb)
  : sockinet_base(sb), osockstream(this)
{

}

osockinet::~osockinet ()
{
}

iosockinet::iosockinet(const sockbuf::sockdesc& sd)
  : sockinet_base(sd), iosockstream(this)
{

}


iosockinet::iosockinet(sockbuf::type ty, int proto)
  : sockinet_base(ty, proto), iosockstream(this)
{

}

iosockinet::iosockinet(const sockinetbuf& sb)
  : sockinet_base(sb), iosockstream(this)
{

}

iosockinet::~iosockinet ()
{
}

}
