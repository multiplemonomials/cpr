
/** Test ability of datagram inet sockets to send and recieve data */

#include "../cprerr.h"

#include <iostream>
#include <thread>

int main()
{
	try
	{
		throw cpr::exception(EINVAL);
	}
	catch(cpr::exception & error)
	{
		std::string expected("Invalid argument");
		if(!(error.what() == expected))
		{
			std::cerr << "ERROR: expected error message \"" << expected << "\" but got \"" << error.what() << "\"" << std::endl;
			return 1;
		}
	}	
	
	try
	{
		throw cpr::exception(EINVAL, "unit test");
	}
	catch(cpr::exception & error)
	{
		std::string expected("Invalid argument from unit test");
		if(!(error.what() == expected))
		{
			std::cerr << "ERROR: expected error message \"" << expected << "\" but got \"" << error.what() << std::endl;
			return 1;
		}
	}
	
	try
	{
		throw cpr::exception(EINVAL, "unit test", "this is supposed to happen");
	}
	catch(cpr::exception & error)
	{
		std::string expected("Invalid argument from unit test (this is supposed to happen)");
		if(!(error.what() == expected))
		{
			std::cerr << "ERROR: expected error message \"" << expected << "\" but got \"" << error.what() << std::endl;
			return 1;
		}
	}
}
