// sockunix.h -*- C++ -*- socket library
// Copyright (C) 1992-1996 Gnanasekaran Swaminathan <gs4t@virginia.edu>
//
// Permission is granted to use at your own risk and distribute this software
// in source and  binary forms provided  the above copyright notice and  this
// paragraph are  preserved on all copies.  This software is provided "as is"
// with no express or implied warranty.
//
// Version: 12Jan97 1.11

#ifndef _SOCKUNIX_H
#define	_SOCKUNIX_H

#ifdef WIN32
#error UNIX sockets are not available on Win32 platforms.
#endif

#include <socket++/sockstream.h>

#include <sys/un.h>

namespace socketpp
{
//-----------------------------------------------------------------------------
class sockunixaddr : public sockAddr, public sockaddr_un
{
public:
                     ~sockunixaddr () {}
  		     sockunixaddr (const char* path);
  		     sockunixaddr (const sockunixaddr& suna);
  		     operator void* () const { return addr_un (); }

  sockaddr_un*       addr_un () const { return (sockaddr_un*)this; }
  int                size () const { return sizeof (sockaddr_un); }
  int                family () const { return sun_family; }
  sockaddr*          addr() const {return (sockaddr*) addr_un (); }
};

//-----------------------------------------------------------------------------
class sockunixbuf: public virtual sockbuf 
{
public:

  typedef sockunixaddr	address_t;
	
  enum domain 
	{
		af_unix = AF_UNIX
	};
    
  sockunixbuf (const sockbuf::sockdesc& sd);
	sockunixbuf (const sockunixbuf& su);
  sockunixbuf (sockbuf::type ty, int proto=0);
	
  ~sockunixbuf () {}

  virtual void       bind (sockAddr& sa);
  void               bind (const char* path);

  virtual void       connect (sockAddr& sa);
  void               connect (const char* path);
};

//-----------------------------------------------------------------------------
class sockunix_base : public virtual sockstream_base, public virtual sockunixbuf
{
	protected:
	sockunix_base(const sockunix_base& otherbase) :  sockbuf(otherbase), sockunixbuf(otherbase)
	{
	
	}
	
	public:
	sockunix_base(const sockbuf::sockdesc& sd) : sockbuf(sd), sockunixbuf(sd)
	{
	
	}
	
	sockunix_base(const sockunixbuf& su) : sockbuf(su), sockunixbuf(su)
	{
	
	}
	
	sockunix_base(sockbuf::type ty, int proto=0) : sockbuf(af_unix, ty, proto), sockunixbuf(ty, proto)
	{
	
	}
	
};

//-----------------------------------------------------------------------------
class isockunix: public virtual sockunix_base, public isockstream
{
public:
  isockunix(const sockbuf::sockdesc& sd);
  isockunix(const sockunixbuf& sb);
  isockunix(sockbuf::type ty=sockbuf::sock_stream, int proto=0);
  isockunix(const isockunix& othersock)
  :sockbuf(othersock),
  sockunixbuf(othersock),
  sockunix_base(othersock)
  {
  
  }
  
	~isockunix();      
};

//-----------------------------------------------------------------------------
class osockunix: public virtual sockunix_base, public osockstream
{
public:
  osockunix(const sockbuf::sockdesc& sd);
  osockunix(const sockunixbuf&);
 	osockunix(sockbuf::type ty=sockbuf::sock_stream, int proto=0);
  osockunix(const osockunix& othersock)
  :sockbuf(othersock),
  sockunixbuf(othersock),
  sockunix_base(othersock)
  {
  
  }
 	
  ~osockunix();     
};

//-----------------------------------------------------------------------------
class iosockunix: public virtual sockunix_base, public iosockstream
{
public:
  iosockunix(const sockbuf::sockdesc& sd);
	iosockunix(const sockunixbuf& sb);
  iosockunix(sockbuf::type ty=sockbuf::sock_stream, int proto=0);
  iosockunix(const iosockunix& othersock)
  :sockbuf(othersock),
  sockunixbuf(othersock),
  sockunix_base(othersock)
  {
  
  }
  
 	~iosockunix();
};

}

#endif	// _SOCKUNIX_H
