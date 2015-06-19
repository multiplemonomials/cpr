#include "socketerr.h"
#include "ossock.h"

namespace socketpp
{

bool socket_error_category_t::isWSAError(int code) const
{
#ifndef USE_WINSOCK
	return false;
#else
	//we need some way of seperating MinGW POSIX error codes from WSA ones
	//according to MSDN, all WSA error codes except 4 are between 10004 and 11031
	if(code == WSA_INVALID_HANDLE || code==WSA_NOT_ENOUGH_MEMORY
			||code == WSA_INVALID_PARAMETER || code== WSA_OPERATION_ABORTED
			|| (code > 10004 && code < 11031))
	{
		return true;
	}
	return false;
#endif
}


err::err(int e, std::string operation, std::string specification)
: errCode(e, socket_error_category_t::socket_error_category)
{
	if(operation != "")
	{
		text += " from ";
		text += operation;
	}
	
	if (specification != "") 
	{
		text += " (";
		text += specification;
		text += ")";
	}
}

const char* err::what() const noexcept
{
	return (text == "" ? errCode.message() : errCode.message() + text).c_str();
}

//statically initialized class variable
socket_error_category_t socket_error_category_t::socket_error_category;

/*
 * MinGW and Winsock share about 2/3 of their error macros.
 * However, the MinGW and Winsock ones have different values.
 * So, we have to account for both.
 */

bool sockerr_io(int err)
// recoverable io error.
{
  switch (err)
  {
#ifdef USE_WINSOCK_ERRORS
  case WSAEINPROGRESS:
  case WSAEALREADY:
#endif
#ifdef USE_POSIX_MINGW_ERRORS
  case EINPROGRESS:
  case EALREADY:
#endif
	return true;
  }
  return false;
}

bool sockerr_arg(int err)
// recoverable argument error.
{
	switch (err)
	{
#ifdef USE_WINSOCK_ERRORS
	case WSAENOTSOCK:
	case WSAEDESTADDRREQ:
	case WSAEMSGSIZE:
	case WSAEPROTOTYPE:
	case WSAENOPROTOOPT:
	case WSAEPROTONOSUPPORT:
	case WSAEOPNOTSUPP:
	case WSAEAFNOSUPPORT:
	case WSAESOCKTNOSUPPORT:
	case WSAEPFNOSUPPORT:
#endif
#ifdef USE_POSIX_MINGW_ERRORS
	case ENOTSOCK:
	case EDESTADDRREQ:
	case EMSGSIZE:
	case EPROTOTYPE:
	case ENOPROTOOPT:
	case EPROTONOSUPPORT:
	case EOPNOTSUPP:
	case EAFNOSUPPORT:
#endif
#ifdef USE_POSIX_ONLY_ERRORS
	case ESOCKTNOSUPPORT:
	case EPFNOSUPPORT:
#endif
		return true;
	}
  switch (err) {
    return true;
  }
  return false;
}

bool sockerr_op(int err)
// operational error encountered 
{
	switch (err)
	{
#ifdef USE_WINSOCK_ERRORS
	case WSAENETDOWN:
	case WSAENETUNREACH:
	case WSAENETRESET:
	case WSAECONNABORTED:
	case WSAECONNRESET:
	case WSAENOBUFS:
	case WSAETIMEDOUT:
	case WSAECONNREFUSED:
	case WSAELOOP:
	case WSAENAMETOOLONG:
	case WSAENOTEMPTY:
	case WSAESHUTDOWN:
	case WSAETOOMANYREFS:
	case WSAEHOSTDOWN:
	case WSAEPROCLIM:
	case WSAEUSERS:
	case WSAEDQUOT:
#endif
#ifdef USE_POSIX_MINGW_ERRORS
	case ENETDOWN:
	case ENETUNREACH:
	case ENETRESET:
	case ECONNABORTED:
	case ECONNRESET:
	case ENOBUFS:
	case ETIMEDOUT:
	case ECONNREFUSED:
	case ELOOP:
	case ENAMETOOLONG:
	case ENOTEMPTY:
	case EHOSTUNREACH:
#endif
#ifdef USE_POSIX_ONLY_ERRORS
	case ESHUTDOWN:
	case ETOOMANYREFS:
	case EHOSTDOWN:
#	if defined(EPROCLIM) // LN
		case EPROCLIM:
#	endif
	case EUSERS:
	case EDQUOT:
#endif
		return true;
	}
  switch (err) {

    return true;
  }
  return false;
}

bool sockerr_conn(int err)
// return true if err is EISCONN, ENOTCONN, ECONNRESET, ECONNREFUSED,
// ETIMEDOUT, or EPIPE
{
	switch (err)
	{
#ifdef USE_WINSOCK_ERRORS
	case WSAEISCONN:
	case WSAENOTCONN:
	case WSAECONNRESET:
	case WSAECONNREFUSED:
	case WSAETIMEDOUT:
#endif
#ifdef USE_POSIX_MINGW_ERRORS
	case EPIPE:
	case EISCONN:
	case ENOTCONN:
	case ECONNRESET:
	case ECONNREFUSED:
	case ETIMEDOUT:
#endif
		return true;
	}
  return false;
}

bool sockerr_addr(int err)
// return true if err is EADDRINUSE or EADDRNOTAVAIL
{
  switch (err)
  {
#ifdef USE_WINSOCK_ERRORS
  case WSAEADDRINUSE:
  case WSAEADDRNOTAVAIL:
#endif
#ifdef USE_POSIX_MINGW_ERRORS
  case EADDRINUSE:
  case EADDRNOTAVAIL:
#endif
    return true;
  }
  return false;
}

bool sockerr_benign(int err)
// return true if err is EINTR, EWOULDBLOCK, or EAGAIN
{
	switch (err)
	{
#ifdef USE_WINSOCK_ERRORS
	case WSAEWOULDBLOCK:
	case WSAEINTR:
#endif
#ifdef USE_POSIX_MINGW_ERRORS
	case EWOULDBLOCK:
#endif
#ifdef USE_POSIX_ONLY_ERRORS
	// On FreeBSD (and probably on Linux too)
	// EAGAIN has the same value as EWOULDBLOCK
	#if EAGAIN != EWOULDBLOCK
		case EAGAIN:
	#endif
#endif
		return true;
	}
	return false;
}

//takes an error code in int form and determines which category it belongs in
std::error_condition socket_error_category_t::default_error_condition(int ev) const noexcept
{
	if (ev == 0)
	{
	  return std::error_condition(socket_errc::success);
	}
	else if(sockerr_benign(ev))
	{
	  return std::error_condition(socket_errc::benign);
	}
	else if(sockerr_addr(ev))
	{
	  return std::error_condition(socket_errc::address);
	}
	else if(sockerr_conn(ev))
	{
	  return std::error_condition(socket_errc::connection);
	}
	else if(sockerr_op(ev))
	{
	  return std::error_condition(socket_errc::operational);
	}
	else if(sockerr_arg(ev))
	{
	  return std::error_condition(socket_errc::argument);
	}
	else if(sockerr_io(ev))
	{
	  return std::error_condition(socket_errc::io);
	}
	else
	{
	  return std::error_condition(socket_errc::other);
	}
}

std::error_condition make_error_condition(socket_errc e)
{
    return std::error_condition(static_cast<int>(e), socket_error_category_t::socket_error_category);
}

}
