// pipestream.h -*- C++ -*- socket library
// Copyright (C) 1992-1996 Gnanasekaran Swaminathan <gs4t@virginia.edu>
//
// Permission is granted to use at your own risk and distribute this software
// in source and  binary forms provided  the above copyright notice and  this
// paragraph are  preserved on all copies.  This software is provided "as is"
// with no express or implied warranty.
//
// Version: 12Jan97 1.11

#ifndef _PIPESTREAM_H
#define	_PIPESTREAM_H

#ifdef WIN32
#error Pipe streams are not available on Win32 platforms.
#endif

#include <socket++/sockstream.h>
#include <socket++/sockunix.h>

namespace cpr
{

std::pair<socketpp::iosockunix, socketpp::iosockunix> make_socketpair();

namespace detail
{
socketpp::sockbuf* createpipestream(const char* cmd, int mode);
}

class iopipestream: public socketpp::sockbuf, public virtual std::iostream
{
public:
    iopipestream(const char* cmd);
    virtual ~iopipestream()
    {
    
    }
};

class ipipestream: public socketpp::sockbuf, public virtual std::istream
{
public:
    ipipestream(const char* cmd);
    virtual ~ipipestream()
    {
    
    }
};

class opipestream: public socketpp::sockbuf, public virtual std::ostream
{
public:
    opipestream(const char* cmd);
    virtual ~opipestream()
    {
    
    }
};

}

#endif	// _PIPESTREAM_H
