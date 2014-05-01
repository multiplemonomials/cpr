
#ifndef CPRERR_H_
#define CPRERR_H_

#include <system_error>
#include <cstring>

namespace cpr
{

//cprerr.h: cpr exception classes

enum class cpr_errc {success=0, io, benign, argument, address, connection, operational, other};

}

namespace std 
{
	template<> struct is_error_condition_enum<cpr::cpr_errc> : public true_type {};
}

namespace cpr
{

//custom socket error category
//for our own "domain" of errors
class cpr_error_category_t : public std::error_category 
{
public:
  virtual const char* name() const noexcept
  { 
		return "CPR Error"; 
  }
  
  static cpr_error_category_t cpr_error_category;
  
  virtual std::error_condition default_error_condition(int ev) const noexcept;
  
  virtual bool equivalent (const std::error_code& code, int condition) const noexcept
  {
    return *this==code.category() && static_cast<int>(default_error_condition(code.value()).value())==condition;
  }
  
  virtual std::string message(int ev) const 
  {
		//we'll be working with errors from the OS
		return std::string(strerror(ev));
  }
};

// make_error_condition overload to generate custom conditions:
std::error_condition make_error_condition(cpr_errc e);


//exception that is thrown from socket++ functions
class exception : std::exception 
{
  std::error_code 	err;
  std::string 		text;
  
public:
  
  /**
  * Construct from OS error, where it happened, and possibly what happened
  * 
  * e.g. cprerr(EINVAL, "bind", "socket in wrong state)
  */
  exception(int e, const char *operation = NULL, const char *specification = NULL)
  : err(e, cpr_error_category_t::cpr_error_category) 
  {
		if (operation != NULL)
			text = operation;
		
		if (specification != NULL) 
		{
			text += "(";
			text += specification;
			text += ")";
		}
  }
  
  exception(int e, const std::string &operation)
  : err (e, cpr_error_category_t::cpr_error_category),
	text(operation)
  {
  }
  
  exception(const exception &) = default;
  
  virtual const char* what() const noexcept
  {
		return (err.message() + " from " + text).c_str(); 
  }
	
  const char* operation() const 
  { 
		return text.c_str(); 
  }

  int get_errno () const 
  { 
		return err.value();
  } // LN
    
	// non-blocking and interrupt io recoverable error.
  bool io() const 
  {
		return err.default_error_condition() == std::error_condition(cpr_errc::io);
  }
  
  /// incorrect argument supplied. recoverable error.
  bool arg() const
  {
		return err.default_error_condition() == std::error_condition(cpr_errc::argument);
  }
  
  /// operational error. recovery difficult.
  bool op() const
  {
		return err.default_error_condition() == std::error_condition(cpr_errc::operational);
  }

	/// connection error
  bool conn() const
  {
		return err.default_error_condition() == std::error_condition(cpr_errc::connection);
  }
  
  /// address error
  bool addr() const   
  {
		return err.default_error_condition() == std::error_condition(cpr_errc::address);
  }
  
  /// recoverable read/write error like EINTR etc.
  bool benign() const
  {
		return err.default_error_condition() == std::error_condition(cpr_errc::benign);
  }
  
};

#ifdef WIN32
//define posix error codes as their Windows equivalents
#	define EWOULDBLOCK			WSAEWOULDBLOCK
#	define EINPROGRESS			WSAEINPROGRESS
#	define EALREADY					WSAEALREADY
#	define ENOTSOCK					WSAENOTSOCK
#	define EDESTADDRREQ			WSAEDESTADDRREQ
#	define EMSGSIZE					WSAEMSGSIZE
#	define EPROTOTYPE				WSAEPROTOTYPE
#	define ENOPROTOOPT			WSAENOPROTOOPT
#	define EPROTONOSUPPORT	WSAEPROTONOSUPPORT
#	define ESOCKTNOSUPPORT	WSAESOCKTNOSUPPORT
#	define EOPNOTSUPP				WSAEOPNOTSUPP
#	define EPFNOSUPPORT			WSAEPFNOSUPPORT
#	define EAFNOSUPPORT			WSAEAFNOSUPPORT
#	define EADDRINUSE				WSAEADDRINUSE
#	define EADDRNOTAVAIL		WSAEADDRNOTAVAIL
#	define ENETDOWN					WSAENETDOWN
#	define ENETUNREACH			WSAENETUNREACH
#	define ENETRESET				WSAENETRESET
#	define ECONNABORTED			WSAECONNABORTED
#	define ECONNRESET				WSAECONNRESET
#	define ENOBUFS					WSAENOBUFS
#	define EISCONN					WSAEISCONN
#	define ENOTCONN					WSAENOTCONN
#	define ESHUTDOWN				WSAESHUTDOWN
#	define ETOOMANYREFS			WSAETOOMANYREFS
#	define ETIMEDOUT				WSAETIMEDOUT
#	define ECONNREFUSED			WSAECONNREFUSED
#	define ELOOP						WSAELOOP
#	define EHOSTDOWN				WSAEHOSTDOWN
#	define EHOSTUNREACH			WSAEHOSTUNREACH
#	define EPROCLIM					WSAEPROCLIM
#	define EUSERS						WSAEUSERS
#	define EDQUOT						WSAEDQUOT
#	define EISCONN					WSAEISCONN
#	define ENOTCONN					WSAENOTCONN
#	define ECONNRESET				WSAECONNRESET
#	define ECONNREFUSED			WSAECONNREFUSED
#	define ETIMEDOUT				WSAETIMEDOUT
#	define EADDRINUSE				WSAEADDRINUSE
#	define EADDRNOTAVAIL		WSAEADDRNOTAVAIL
#	define EWOULDBLOCK			WSAEWOULDBLOCK
#endif //WIN32

}

#endif //CPRERR_H_
