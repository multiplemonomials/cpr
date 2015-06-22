
#ifndef SOCKETERR_H_
#define SOCKETERR_H_

#include <system_error>
#include <cstring>
#include <type_traits>
#include "ossock.h"

namespace socketpp
{

//socketerr.h: socket++ err classes

enum class socket_errc {success=0, io, benign, argument, address, connection, operational, other};

}

namespace std 
{
	template<> struct is_error_condition_enum<socketpp::socket_errc> : public true_type {};
}

namespace socketpp
{

//custom socket error category
//for our own "domain" of errors
class socket_error_category_t : public std::error_category 
{
public:
  virtual const char* name() const noexcept
  { 
		return "Socket++ Error"; 
  }
  
  static socket_error_category_t socket_error_category;
  
  virtual std::error_condition default_error_condition(int ev) const noexcept;
  
  virtual bool equivalent (const std::error_code& code, int condition) const noexcept
  {
    return *this==code.category() && static_cast<int>(default_error_condition(code.value()).value())==condition;
  }
  
  //return true if the provided is a WSA error code
  //otherwise it is a POSIX one
  bool isWSAError(int code) const;

  virtual std::string message(int ev) const 
  {
	  //we'll be working with errors from the OS
	  if(isWSAError(ev))
	  {
		  //use WSA's crazy error-string function
#ifdef USE_WINSOCK
		  char* osMessage = nullptr;
		  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				  nullptr, ev, LANG_USER_DEFAULT, (LPTSTR)&osMessage, 1, nullptr);

		  std::string retval(osMessage);
		  LocalFree(osMessage);
		  return retval;
#endif
	  }
	  else //use strerror
	  {
		  return std::string(strerror(ev));
	  }
	  
	  //should never get here.
	  //This is only here to stop te compiler warning when USE_WINSOCK is not defined.
	  return "Error determining the error code type.";
  }

  virtual ~socket_error_category_t(){}
};

// make_error_condition overload to generate custom conditions:
std::error_condition make_error_condition(socket_errc e);


//exception that is thrown from socket++ functions
class err : std::exception 
{
  std::error_code 	errCode;
  std::string 		text;
  
public:
  
  /**
  * Construct from OS error, where it happened, and possibly what happened
  * 
  * e.g. socketerr(EINVAL, "bind", "socket in wrong state")
  * what() would print "Invalid argument from bind (socket in wrong state)"
  */
  err(int e, std::string operation = "", std::string specification = "");
  
  err(const err &) = default;
  
  virtual const char* what() const noexcept;
	
  const char* operation() const 
  { 
		return text.c_str(); 
  }

  int get_errno () const 
  { 
		return errCode.value();
  } // LN
    
	// non-blocking and interrupt io recoverable error.
  bool io() const 
  {
		return errCode.default_error_condition() == std::error_condition(socket_errc::io);
  }
  
  /// incorrect argument supplied. recoverable error.
  bool arg() const
  {
		return errCode.default_error_condition() == std::error_condition(socket_errc::argument);
  }
  
  /// operational error. recovery difficult.
  bool op() const
  {
		return errCode.default_error_condition() == std::error_condition(socket_errc::operational);
  }

	/// connection error
  bool conn() const
  {
		return errCode.default_error_condition() == std::error_condition(socket_errc::connection);
  }
  
  /// address error
  bool addr() const   
  {
		return errCode.default_error_condition() == std::error_condition(socket_errc::address);
  }
  
  /// recoverable read/write error like EINTR etc.
  bool benign() const
  {
		return errCode.default_error_condition() == std::error_condition(socket_errc::benign);
  }
};

}

#endif //SOCKETERR_H_
