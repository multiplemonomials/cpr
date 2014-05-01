// sockinet.h -*- C++ -*- socket library
// Copyright (C) 1992-1996 Gnanasekaran Swaminathan <gs4t@virginia.edu>
//
// Permission is granted to use at your own risk and distribute this software
// in source and  binary forms provided  the above copyright notice and  this
// paragraph are  preserved on all copies.  This software is provided "as is"
// with no express or implied warranty.
//
// Version: 12Jan97 1.11

#ifndef _SOCKINET_H
#define	_SOCKINET_H

#include <socket++/sockstream.h>
#ifndef WIN32
#	include <netinet/in.h>
#endif // !WIN32

namespace socketpp
{

//address object for inet sockets
class sockinetaddr: public sockAddr, public sockaddr_in
{
protected:
  void                setport (const char* sn, const char* pn="tcp");
  void                setaddr (const char* hn);
public:
                      ~sockinetaddr () {}
  		      sockinetaddr ();
  		      sockinetaddr (unsigned long addr, int port_no=0);
  		      sockinetaddr (const char* host_name, int port_no=0);
  		      sockinetaddr (unsigned long addr, const char* service_name, const char* protocol_name="tcp");
  		      sockinetaddr (const char* host_name, const char* service_name, const char* protocol_name="tcp");
  		      sockinetaddr (const sockinetaddr& sina);    

                      operator void* () const { return addr_in (); }

  sockaddr_in*        addr_in () const { return (sockaddr_in*) this; }
  int                 size  () const { return sizeof (sockaddr_in); }
  int                 family() const { return sin_family; }
  sockaddr*           addr  () const { return (sockaddr*) addr_in (); }

  int                 getport    () const;
  const char*         gethostname() const;
};

//extended buffer class for inet sockets
class sockinetbuf: public sockbuf 
{
public:

  typedef sockinetaddr address_t;

  enum domain
  {
	af_inet = AF_INET
  };
   
  //construct buffer from already-existing socket
  sockinetbuf(const sockbuf::sockdesc& sd);
  
  //copy constructor
  sockinetbuf(const sockinetbuf& si): sockbuf (si) {}
  
  
  sockinetbuf(sockbuf::type ty, int proto=0);
  
  sockinetbuf& operator=(const sockinetbuf& si);
  virtual ~sockinetbuf () {}  

  sockinetaddr        localaddr() const;
  int                 localport() const;
  const char*         localhost() const;
    
  sockinetaddr        peeraddr() const;
  int                 peerport() const;
  const char*         peerhost() const;

  int                 bind_until_success(int portno);

  virtual void        bind(sockAddr& sa);
  void		      	  	bind(int port_no=0);  // addr is assumed to be INADDR_ANY
                                            // and thus defaults to local host
  void		      	  	bind (unsigned long addr, int port_no);
  void		      	  	bind (const char* host_name, int port_no=0);
  void		      			bind (unsigned long addr, const char* service_name, const char* protocol_name="tcp");
  void		      			bind (const char* host_name, const char* service_name, const char* protocol_name="tcp");

  virtual void	      connect (sockAddr& sa);
  void		      			connect (unsigned long addr, int port_no);
  void		      			connect (const char* host_name, int port_no);
  void		      			connect (unsigned long addr, const char* service_name, const char* protocol_name="tcp");
  void		      			connect (const char* host_name, const char* service_name, const char* protocol_name="tcp");

  bool                tcpnodelay () const;
  bool                tcpnodelay (bool set) const;
};

class sockinet_base : public virtual sockstream_base, public sockinetbuf
{
	public:
	sockinet_base(const sockbuf::sockdesc& sd) : sockinetbuf(sd) 
	{
	
	}
	
	public:
	sockinet_base(const sockinetbuf& si) : sockinetbuf(si) 
	{
	
	}
	
	public:
	sockinet_base(sockbuf::type ty, int proto=0) : sockinetbuf(ty, proto) 
	{
	
	}
	
};

class isockinet: public sockinet_base, public isockstream
{
public:
  isockinet (const sockbuf::sockdesc& sd);
	isockinet (const sockinetbuf& sb);
  isockinet (sockbuf::type ty=sockbuf::sock_stream, int proto=0);
  ~isockinet ();      
};

class osockinet: public sockinet_base, public osockstream
{
public:
  osockinet(const sockbuf::sockdesc& sd);
	osockinet(const sockinetbuf& sb);
	osockinet(sockbuf::type ty=sockbuf::sock_stream, int proto=0);
  virtual ~osockinet ();      
};

class iosockinet:  public sockinet_base, public iosockstream
{
public:
  iosockinet (const sockbuf::sockdesc& sd);
	iosockinet (const sockinetbuf& sb);
  iosockinet (sockbuf::type ty=sockbuf::sock_stream, int proto=0);
	virtual ~iosockinet ();     
};

}

#endif	// _SOCKINET_H
