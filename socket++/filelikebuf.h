

#ifndef _FILELIKEBUF_H
#define	_FILELIKEBUF_H

#include "socketerr.h"
#include "ossock.h"

#include <iostream> // must be ANSI compatible
#include <cstddef>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <cstdio>
#include <memory>

namespace socketpp
{

// buffer for pipes which read like files.
class filelikebuf: public std::streambuf
{
public:

  typedef char          char_type;
  typedef std::streampos     pos_type;
  typedef std::streamoff     off_type;
  typedef int           int_type;
  typedef int           seekdir;
  
  //OS handle to the thing
#ifdef USE_WINSOCK
  typedef PHANDLE filedesc;
#else
  typedef int filedesc;
#endif

  //internal impl object that stores the file info
  //allows the outer object to be passed by value
  struct filecnt
  {
    filedesc	file;
    void*		gend; // end of input buffer
    void*		pend; // end of output buffer

    filecnt(filedesc s)
	: file(s),
	gend(0),
	pend(0)
	{}

    filecnt(filecnt &) = delete;
  };


protected:
  std::shared_ptr<filecnt>		rep;

  std::string		sockname; // name of filelikebuf - Herbert Straub

  virtual int           sync ();
  
  virtual int           showmanyc () const;
  virtual std::streamsize    xsgetn (char_type* s, std::streamsize n);
  virtual int_type      underflow ();
  virtual int_type      uflow ();

  virtual int_type      pbackfail (int_type c = EOF);

  virtual std::streamsize    xsputn (const char_type* s, std::streamsize n);
  virtual int_type      overflow (int_type c = EOF);

public:

  ///create from existing OS handle
  ///WARNING: Don't use this constructor to create multiple filelikebuf's from a handle, it will cause the file to be closed multiple times!
  ///Use the copy constructor instead
  filelikebuf(const filedesc& fd);

  //assign data of filelikebuf
  filelikebuf(const filelikebuf&);
  
  virtual 		~filelikebuf ();

  filedesc                   fd () const { return rep->file; }
  int                   pubsync () { return sync(); }
    
  int   		read	(void* buf, int len);
    
  int			write	(const void* buf, int len);

  inline void		setname	  (const char *name);
  inline void		setname	  (const std::string &name);
  inline const std::string&	getname	  ();
};

//holds functions common between every sockstream
//as well as the buffer
class filelikestream_base : public virtual std::ios
{
protected:
	filelikestream_base(){};
  public:
	filelikestream_base(const filelikestream_base&) = delete;

	
  filelikebuf*	rdbuf()
  {
		return (filelikebuf*)rdbuf();
  }
  
  virtual ~filelikestream_base()
  {

  }
};

//base class for socket input streams
class ifilestream: public std::istream, public virtual filelikestream_base
{
protected:
	ifilestream();
	
public:
	ifilestream(filelikebuf* sb): filelikestream_base(), std::istream(sb){}
	virtual ~ifilestream () {}
};

//base class for socket output streams
class ofilestream: public std::ostream, public virtual filelikestream_base
{
protected:
    ofilestream();
	
public:
	ofilestream(filelikebuf* sb): filelikestream_base(), std::ostream(sb) {}
	virtual ~ofilestream () {}
};

//base class for socket iostreams
class iofilestream: public std::iostream, public virtual filelikestream_base
{
protected:
    iofilestream();

public:
	iofilestream(const iofilestream&) = delete;

    iofilestream(filelikebuf* sb):  filelikestream_base(), std::iostream(sb) {}
  virtual ~iofilestream () {}
};

// inline

void filelikebuf::setname (const char *name)
{
	sockname = name;
}
void filelikebuf::setname (const std::string &name)
{
	sockname = name;
}
const std::string& filelikebuf::getname ()
{
	return sockname;
}

}

#endif	// _FILELIKEBUF_H
