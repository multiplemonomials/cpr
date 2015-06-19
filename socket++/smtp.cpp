// smtp.C -*- C++ -*- socket library
// Copyright (C) 1992-1996 Gnanasekaran Swaminathan <gs4t@virginia.edu>
//
// Permission is granted to use at your own risk and distribute this software
// in source and  binary forms provided  the above copyright notice and  this
// paragraph are  preserved on all copies.  This software is provided "as is"
// with no express or implied warranty.
//
// Version: 12Jan97 1.11

#include "smtp.h"
#include <stdlib.h>
#include <string.h>

namespace socketpp
{

using namespace std;

void smtp::smtpbuf::get_response ()
     // get all the response that one can get and send all of them to output
{
  // if output is 0, then we trash data.
  while (underflow () != EOF) {
    int n = in_avail ();
    if (n < 5)
      continue;

    // data is of this form: 221 repsonse <CRLF> or 221-response <CRLF>
    char* q = gptr ();
    char* p = q;

    // zap upto <CRLF>
    for (int i = 2; i <= n; i++, p++)
      if (*p == '\r' && *(p+1) == '\n') {
	if (output)
	  output->write (q, i);
	gbump (i);
	break;
      }
    
    if (q [3] != '-')
      break;
  }  
}

void smtp::smtpbuf::send_cmd (const char* cmd, const char* s, const char* p)
{
  xsputn (cmd, ::strlen (cmd));
  if (s)
    xsputn (s, ::strlen (s));
  if (p)
    xsputn (p, ::strlen (p));
  xsputn ("\r\n", 2);
  sync ();
  
  if (output)
    get_response ();
}

void smtp::smtpbuf::helo ()
{
  if (output) 
    get_response ();
  send_cmd ("HELO ", localhost ());
}

void smtp::smtpbuf::mail (const char* reverse_path)
{
  if (reverse_path)
    send_cmd ("MAIL FROM:<", reverse_path, ">");
  else
    send_cmd ("MAIL FROM:<>");
}

void smtp::smtpbuf::rcpt (const char* forward_path)
{
  if (forward_path)
    send_cmd ("RCPT TO:<", forward_path, ">");
}

void smtp::smtpbuf::help (const char* s)
{
  send_cmd ("HELP ", s);
}

void smtp::smtpbuf::send_buf (const char* buf, int len)
{
  if (buf == 0 || len <= 0)
    return;

  // send line by line
  const unsigned char* p = (const unsigned char*) buf;

  if (*p == '.')
    sputc ((unsigned int) '.');

  for (int i = 0; i < len; i++, p++) {
    if (*p == '\n') {
      sputc ((unsigned int) '\r');
      sputc (*p);
      if (*(p+1) == '.')
	sputc ((unsigned int) '.');
    } else
      sputc (*p);
  }
}

void smtp::smtpbuf::data (const char* buf, int len)
{
  data ();
  send_buf (buf, len);
  xsputn ("\r\n.\r\n", 5);
  sync ();

  if (output)
    get_response ();
}

void smtp::smtpbuf::data (const char* filename)
{
  data ();

	#ifdef USE_WINSOCK
		HANDLE fd = nullptr;
		DWORD rcnt;
	#else
		int  fd = 0;
		int  rcnt;
	#endif
  char buf [1024];
	#ifdef USE_WINSOCK
		OFSTRUCT fileInfo;
		memset(&fileInfo, 0, sizeof(fileInfo));
		if (filename == nullptr || (fd = ::CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr)) == INVALID_HANDLE_VALUE)
		{
			fd = 0;
		}
	#else
		if (filename == 0 || (fd = ::open(filename, O_RDONLY )) == -1)
			fd = 0;
	#endif
	
	#ifdef USE_WINSOCK
		while(ReadFile(fd, buf, sizeof(buf), &rcnt, nullptr))
	#else
		while ((rcnt = ::read (fd, buf, sizeof(buf))) > 0)
	#endif
    send_buf (buf, rcnt);

  xsputn ("\r\n.\r\n", 5);
  sync ();
	
	#ifdef USE_WINSOCK
		CloseHandle(fd);
	#endif

  if (output)
    get_response ();
}

int smtp::get_response (char* buf, int len)
     // same as get line except what it returns
     // return 1 if output continues after this line
     // return 0 if output has terminated
{
  if (len < 8) {
    this->getline (buf, len);
    return 0;
  }

  buf [3] = 0;
  this->getline (buf, len);
  return buf [3] == '-';
}
  
ostream& operator << (ostream& output, smtp& s)
{
  char buf [1024];
  int  cont = 1;
  while (cont) {
    cont = s.get_response (buf, 1024);
    output << buf << endl;
  }
  return output;
}


void smtp::smtpbuf::serve_clients (int portno)
{
}

}
