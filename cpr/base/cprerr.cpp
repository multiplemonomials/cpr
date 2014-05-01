#include <base/cprerr.h>

#ifdef _WIN32
#include <Wininet.h>
#endif

namespace cpr
{

//statically initialized class variable
cpr_error_category_t cpr_error_category_t::cpr_error_category;

bool sockerr_io(int err)
// recoverable io error.
{
  switch (err) {
  case EWOULDBLOCK:
  case EINPROGRESS:
  case EALREADY:
    return true;
  }
  return false;
}

bool sockerr_arg(int err)
// recoverable argument error.
{
  switch (err) {
  case ENOTSOCK:
  case EDESTADDRREQ:
  case EMSGSIZE:
  case EPROTOTYPE:
  case ENOPROTOOPT:
  case EPROTONOSUPPORT:
  case ESOCKTNOSUPPORT:
  case EOPNOTSUPP:
  case EPFNOSUPPORT:
  case EAFNOSUPPORT:
  case EADDRINUSE:
  case EADDRNOTAVAIL:
    return true;
  }
  return false;
}

bool sockerr_op(int err)
// operational error encountered 
{
  switch (err) {
  case ENETDOWN:
  case ENETUNREACH:
  case ENETRESET:
  case ECONNABORTED:
  case ECONNRESET:
  case ENOBUFS:
  case EISCONN:
  case ENOTCONN:
  case ESHUTDOWN:
  case ETOOMANYREFS:
  case ETIMEDOUT:
  case ECONNREFUSED:
  case ELOOP:
  case ENAMETOOLONG:
  case EHOSTDOWN:
  case EHOSTUNREACH:
  case ENOTEMPTY:
#	if !defined(__linux__) // LN
  case EPROCLIM:
#	endif
  case EUSERS:
  case EDQUOT:
    return true;
  }
  return false;
}

bool sockerr_conn(int err)
// return true if err is EISCONN, ENOTCONN, ECONNRESET, ECONNREFUSED,
// ETIMEDOUT, or EPIPE
{
  switch (err) {
  case EISCONN:
  case ENOTCONN:
  case ECONNRESET:
  case ECONNREFUSED:
  case ETIMEDOUT:
	case EPIPE:
    return true;
  }
  return false;
}

bool sockerr_addr(int err)
// return true if err is EADDRINUSE or EADDRNOTAVAIL
{
  switch (err) {
  case EADDRINUSE:
  case EADDRNOTAVAIL:
    return true;
  }
  return false;
}

bool sockerr_benign(int err)
// return true if err is EINTR, EWOULDBLOCK, or EAGAIN
{
  switch (err) {
  case EINTR:
  case EWOULDBLOCK:
// On FreeBSD (and probably on Linux too) 
// EAGAIN has the same value as EWOULDBLOCK
#if !defined(__linux__) && !(defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__APPLE__) || defined(__CYGWIN__)) // LN
  case EAGAIN:
#endif
    return true;
  }
  return false;
}

//takes an error code in int form and determines which category it belongs in
std::error_condition cpr_error_category_t::default_error_condition(int ev) const noexcept
{
	if (ev == 0)
	{
	  return std::error_condition(cpr_errc::success);
	}
	else if(sockerr_benign(ev))
	{
	  return std::error_condition(cpr_errc::benign);
	}
	else if(sockerr_addr(ev))
	{
	  return std::error_condition(cpr_errc::address);
	}
	else if(sockerr_conn(ev))
	{
	  return std::error_condition(cpr_errc::connection);
	}
	else if(sockerr_op(ev))
	{
	  return std::error_condition(cpr_errc::operational);
	}
	else if(sockerr_arg(ev))
	{
	  return std::error_condition(cpr_errc::argument);
	}
	else if(sockerr_io(ev))
	{
	  return std::error_condition(cpr_errc::io);
	}
	else
	{
	  return std::error_condition(cpr_errc::other);
	}
}

std::error_condition make_error_condition(cpr_errc e) 
{
    return std::error_condition(static_cast<int>(e), cpr_error_category_t::cpr_error_category);
}

}
