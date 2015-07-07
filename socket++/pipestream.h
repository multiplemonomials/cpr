// pipestream.h -*- C++ -*- socket library
// Copyright (C) 1992-1996 Gnanasekaran Swaminathan <gs4t@virginia.edu>
//
// Permission is granted to use at your own risk and distribute this software
// in source and  binary forms provided  the above copyright notice and  this
// paragraph are  preserved on all copies.  This software is provided "as is"
// with no express or implied warranty.
//
// Version: 12Jan97 1.11

#ifndef _pipestream_H
#define	_pipestream_H

#include "filelikebuf.h"

#include <memory>

namespace socketpp
{

//extended buffer class for inet sockets
class pipestreambuf : public filelikebuf
{
protected:
	//construct from file handle
	//only called from createpipe()
	pipestreambuf(filedesc file);

public:

	//copy constructor
	pipestreambuf(const pipestreambuf& pbuf): filelikebuf (pbuf) {}

	pipestreambuf& operator=(const pipestreambuf& si);
	virtual ~pipestreambuf () {}

	bool inherit();
	bool inherit(bool toSet);

	bool close_on_exec();
	bool close_on_exec(bool toSet);
};

class pipestream_base : public virtual filelikestream_base, public pipestreambuf
{
protected:
	pipestream_base(const filelikebuf::filedesc& sd) : pipestreambuf(sd)
	{
	
	}
	
public:
	pipestream_base(const pipestream_base&) = delete;
	
};

class ipipestream: public pipestream_base, public ifilestream
{
private:
  ipipestream (const filelikebuf::filedesc& sd);
public:
	ipipestream (const ipipestream&) = delete;
  virtual ~ipipestream ();
};

class opipestream: public pipestream_base, public ofilestream
{
private:
	opipestream (const filelikebuf::filedesc& sd);
public:
	opipestream (const opipestream&) = delete;
  virtual ~opipestream ();
};

typedef std::pair<std::shared_ptr<ipipestream>, std::shared_ptr<opipestream>> pipepair;

///creates a "local" pipe which can transfer data within the process or its children.
pipepair createpipe();

}


#endif	// _pipestream_H
