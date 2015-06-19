// sockunix.cpp  -*- C++ -*- socket library
// Copyright (C) 2002 Herbert Straub
//
// sockunix.C -*- C++ -*- socket library
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
//  and in the second step call ios::init (sockinetbuf *) and iosockstream::init ...
//  The constructors of isockunix, osockunix and iosockunix are changed.
//
// Version: 17 Feb 2014
// Changed it so that all of the sockstreams ARE their buffers through inheritance
// There is no need to call somesock->foo() or somesock.rdbuf()->foo() to access buffer functions, just call somesock.foo()
// --Jamie Smith

#include "ossock.h"

#ifndef USE_WINSOCK

using namespace std;

#include "sockunix.h"

#include <string.h>

namespace socketpp
{

sockunixaddr::sockunixaddr (const char* path)
{
  sun_family = sockunixbuf::af_unix;
  ::strcpy (sun_path, path);
}

sockunixaddr::sockunixaddr (const sockunixaddr& suna)
{
  sun_family = sockunixbuf::af_unix;
  ::strcpy (sun_path, suna.sun_path);
}

sockunixbuf::sockunixbuf (const sockbuf::sockdesc& sd)
  : sockbuf (sd)
{}

sockunixbuf::sockunixbuf (const sockunixbuf& su)
  : sockbuf (su)
{}

sockunixbuf::sockunixbuf (sockbuf::type ty, int proto)
  : sockbuf (af_unix, ty, proto)
{}

void sockunixbuf::bind (sockAddr& sa)
{
  sockbuf::bind (sa);
}

void sockunixbuf::bind (const char* path)
{
  sockunixaddr sa (path);
  bind (sa);
}

void sockunixbuf::connect (sockAddr& sa)
{
  sockbuf::connect (sa);
}

void sockunixbuf::connect (const char* path)
{
  sockunixaddr sa (path);
  connect (sa);
}

isockunix::isockunix(const sockbuf::sockdesc& sd)
  : sockbuf(sd),
  sockunixbuf(sd),
  sockunix_base(sd),
 	isockstream(this)
{

}

isockunix::isockunix(sockbuf::type ty, int proto)
  : sockbuf(af_unix, ty, proto), sockunixbuf(ty, proto), sockunix_base(ty, proto), isockstream(this)
{

}

isockunix::isockunix(const sockunixbuf& sb)
  : sockbuf(sb), sockunixbuf(sb), sockunix_base(sb), isockstream(this)
{

}

isockunix::~isockunix ()
{
}

osockunix::osockunix(const sockbuf::sockdesc& sd)
  : sockbuf(sd),
  sockunixbuf(sd),
  sockunix_base(sd),
	 osockstream(this)
{

}


osockunix::osockunix(sockbuf::type ty, int proto)
  : sockbuf(af_unix, ty, proto), sockunixbuf(ty, proto),  sockunix_base(ty, proto), osockstream(this)
{

}

osockunix::osockunix(const sockunixbuf& sb)
  : sockbuf(sb), sockunixbuf(sb), sockunix_base(sb), osockstream(this)
{

}

osockunix::~osockunix ()
{
}

iosockunix::iosockunix(const sockbuf::sockdesc& sd)
  : sockbuf(sd),
  sockunixbuf(sd),
  sockunix_base(sd), 
  iosockstream(this)
{

}


iosockunix::iosockunix(sockbuf::type ty, int proto)
  : sockbuf(af_unix, ty, proto), sockunixbuf(ty, proto), sockunix_base(ty, proto), iosockstream(this)
{

}

iosockunix::iosockunix(const sockunixbuf& sb)
  : sockbuf(sb), sockunixbuf(sb), sockunix_base(sb), iosockstream(this)
{

}

iosockunix::~iosockunix ()
{
}

} /* namespace socketpp */

#else

#warning Unix Sockets are not supported under Winsock

#endif
