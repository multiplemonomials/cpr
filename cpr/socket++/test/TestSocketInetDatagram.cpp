
/** Test ability of datagram inet sockets to send and recieve data */

#include <cpr/socket++/sockinet.h>

#include <iostream>
#include <thread>

int main()
{
	try
	{
		socketpp::isockinet isin(socketpp::sockbuf::sock_dgram);
		socketpp::osockinet osin(socketpp::sockbuf::sock_dgram);
	
		std::cout << "binding to port 8749" << std::endl;
		isin.bind(8749);
	
		osin.connect("127.0.0.1", 8749);
	
		std::string transmission("foo-bar");
		std::string reception;
	
		osin << transmission.length() << transmission << std::endl;
	
		int n;
		isin >> n;
		std::cout << n << std::endl;
		
		isin >> reception;
		
		std::cout << reception << std::endl;
	
		if(reception == transmission)
		{
			return 0;
		}
		
		std::cerr << "ERROR: Recieved \"" << reception << "\", expected \"" << transmission << "\"" << std::endl;
		return 1;
	}
	catch(socketpp::sockerr & error)
	{
		std::cerr << "Caught sockerr: " << error.what() << std::endl;
		return 1;
	}	
}
