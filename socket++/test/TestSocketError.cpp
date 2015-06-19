
/** Test ability of datagram inet sockets to send and recieve data */

#include "../socketerr.h"

#include <iostream>
#include <thread>

int main()
{
	try
	{
		throw socketpp::err(EINVAL);
	}
	catch(socketpp::err & error)
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
		throw socketpp::err(EINVAL, "unit test");
	}
	catch(socketpp::err & error)
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
		throw socketpp::err(EINVAL, "unit test", "this is supposed to happen");
	}
	catch(socketpp::err & error)
	{
		std::string expected("Invalid argument from unit test (this is supposed to happen)");
		if(!(error.what() == expected))
		{
			std::cerr << "ERROR: expected error message \"" << expected << "\" but got \"" << error.what() << std::endl;
			return 1;
		}
	}
}
