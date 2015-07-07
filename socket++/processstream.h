// pipestream.h -*- C++ -*- socket library
// Copyright (C) 1992-1996 Gnanasekaran Swaminathan <gs4t@virginia.edu>
//
// Permission is granted to use at your own risk and distribute this software
// in source and  binary forms provided  the above copyright notice and  this
// paragraph are  preserved on all copies.  This software is provided "as is"
// with no express or implied warranty.
//
// Version: 12Jan97 1.11

#ifndef _PROCESSTREAM_H
#define	_PROCESSTREAM_H

#include "pipestream.h"

namespace socketpp
{

	pipepair createprocessstream(const char* cmd);
}

#endif	// _PROCESSTREAM_H
