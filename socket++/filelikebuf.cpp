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
// a filelikebuf just like you can listen and talk
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

#include "filelikebuf.h"
#include <sstream>
#include <string>

#ifndef BUFSIZ
#  define BUFSIZ 1024
#endif

namespace socketpp
{

filelikebuf::filelikebuf (const filelikebuf::filedesc& filedescriptor)
: rep(std::make_shared<filecnt>(filedescriptor)),
sockname("filelikebuf " + std::to_string(filedescriptor))
{
  char_type* gbuf = new char_type [BUFSIZ];
  char_type* pbuf = new char_type [BUFSIZ];
  setg (gbuf, gbuf + BUFSIZ, gbuf + BUFSIZ);
  setp (pbuf, pbuf + BUFSIZ);
}	   

filelikebuf::filelikebuf (const filelikebuf& sb)
:rep (sb.rep),
sockname("filelikebuf " + std::to_string(sb.fd()))
{
  // the streambuf::streambuf (const streambuf&) is assumed
  // to have handled pbase () and gbase () correctly.
}


filelikebuf::~filelikebuf ()
{
  overflow(EOF); // flush write buffer
  
  //if the last instance is being destroyed, close socket and destroy object
  if(rep.use_count() == 1)
  {
    delete [] pbase ();
    delete [] eback ();
#ifdef USE_WINSOCK
    	int c = CloseHandle(rep->file);
#else
		int c = close(rep->file);
#endif
    if (c != 0)
		throw err(lastError(), "filelikebuf::~filelikebuf", sockname);
  }
}

int filelikebuf::sync ()
// we never return -1 because we throw socketpp::err
// exception in the event of an error.
{
  if (pptr () && pbase () < pptr () && pptr () <= epptr ()) {
    // we have some data to flush
    try {
      write (pbase (), pptr () - pbase ());
    }
    catch (int wlen)
    {
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
      throw err(lastError(), "filelikebuf::sync", errMessage);
    }

    setp (pbase (), (char_type*) rep->pend);
  }

  // we cannot restore input data back to the socket stream
  // thus we do not do anything on the input stream

  return 0;
}

/// return the number of chars in the input sequence
int filelikebuf::showmanyc () const
{
  if (gptr () && gptr () < egptr ())
    return egptr () - gptr ();
  return 0;
}

filelikebuf::int_type filelikebuf::underflow ()
{
  if (gptr () == 0)
    return EOF; // input stream has been disabled

  if (gptr () < egptr ())
    return (unsigned char) *gptr (); // EOF is a -ve number; make it
                                     // unsigned to be diff from EOF

  int rlen = read(eback (), (char*) rep->gend - (char*) eback ());

  if (rlen == 0)
    return EOF;

  setg (eback (), eback (), eback () + rlen);
  return (unsigned char) *gptr ();
}

filelikebuf::int_type filelikebuf::uflow ()
{
  int_type ret = underflow ();
  if (ret == EOF)
    return EOF;

  gbump (1);
  return ret;
}

std::streamsize filelikebuf::xsgetn(char_type* s, std::streamsize n)
{
  int rval = showmanyc ();
  if (rval >= n) {
    memcpy (s, gptr (), n * sizeof (char_type));
    gbump (n);
    return n;
  }

  memcpy (s, gptr (), rval * sizeof (char_type));
  gbump (rval);

  if (underflow () != EOF)
    return rval + xsgetn (s + rval, n - rval);
  
  return rval;
}

filelikebuf::int_type filelikebuf::pbackfail (int c)
{
  return EOF;
}

/** if pbase () == 0, no write is allowed and thus return EOF.
* if c == EOF, we sync the output and return 0.
* if pptr () == epptr (), buffer is full and thus sync the output,
* insert c into buffer, and return c.
* In all cases, if error happens, throw exception. */
filelikebuf::int_type filelikebuf::overflow (filelikebuf::int_type c)
{
  if (pbase () == 0)
    return EOF;

  if (c == EOF)
    return sync ();

  if (pptr () == epptr ())
    sync ();
  *pptr () = (char_type)c;
  pbump (1);
  return c;
}

std::streamsize filelikebuf::xsputn(const char_type* s, std::streamsize n)
{
  int wval = epptr () - pptr ();
  if (n <= wval) {
    memcpy (pptr (), s, n * sizeof (char_type));
    pbump (n);
    return n;
  }

  memcpy (pptr (), s, wval * sizeof (char_type));
  pbump (wval);
  
  if (overflow () != EOF)
    return wval + xsputn (s + wval, n - wval);

  return wval;
}

int filelikebuf::read (void* buf, int len)
{
	#ifdef USE_WINSOCK
		DWORD rval = 0;
		if(ReadFile(rep->file, buf, len, &rval, nullptr))
	#else
		int rval = 0;
		if ((rval = ::read (rep->file, (char*) buf, len)) == -1)
	#endif
		throw err(errno, "filelikebuf::read", sockname);
	return rval;
}

int filelikebuf::write(const void* buf, int len)
/// upon error, write throws the number of bytes writen so far instead
/// of a socketpp::err.
{
	char *pbuf = (char*) buf;

	int wlen=0;
	while(len>0)
	{
#ifdef USE_WINSOCK
		DWORD wval = 0;
		WriteFile(rep->file, pbuf+wlen, len, &wval, nullptr);
#else
		int	wval = ::write (rep->file, pbuf+wlen, len);
#endif
		if (wval == -1) throw wlen;
		len -= wval;
		wlen += wval;
	}
	return wlen; // == len if every thing is all right
}

}
