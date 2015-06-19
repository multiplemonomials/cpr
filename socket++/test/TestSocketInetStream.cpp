
/** Test ability of datagram inet sockets to send and recieve data */

#include "../sockinet.h"
#include "../socketerr.h"

#include <iostream>
#include <thread>

//function to accept connections on a different thread
void acceptConnection(socketpp::isockinet* socket, socketpp::isockinet** result)
{
	*result = new socketpp::isockinet(socket->accept());
}

int main()
{
	try
	{
		socketpp::isockinet receiver(socketpp::sockbuf::sock_stream);
		socketpp::osockinet osin(socketpp::sockbuf::sock_stream);
	
		std::cout << "binding to port 8749" << std::endl;
		receiver.bind(8749);
		receiver.listen();
		
		socketpp::isockinet * isin = nullptr;
		
		//call accept in a different thread
		std::thread acceptor(&acceptConnection, &receiver, &isin);
	
		osin.connect("127.0.0.1", 8749);
		
		//wait for connection to be accepted
		acceptor.join();
	
		std::string transmission("foo-bar");
		std::string reception;
	
		osin << transmission.length() << transmission << std::endl;
	
		int n;
	
		*isin >> n;
		std::cout << n << std::endl;
		
		*isin >> reception;
		
		std::cout << reception << std::endl;
	
		if(reception == transmission)
		{
			return 0;
		}
		
		std::cerr << "ERROR: Recieved \"" << reception << "\", expected \"" << transmission << "\"" << std::endl;
		return 1;
	}
	catch(socketpp::err & error)
	{
		std::cerr << "Caught sockerr: " << error.what() << std::endl;
		return 1;
	}	
}
