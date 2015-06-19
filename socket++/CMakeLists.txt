add_library(socket++
	echo.cpp
	ftp.cpp
	protocol.cpp
	smtp.cpp
	sockinet.cpp
	sockstream.cpp
	sockunix.cpp
	fork.cpp
	pipestream.cpp
	sig.cpp
	socketerr.cpp)

install (TARGETS socket++ DESTINATION lib)

install (FILES 
	echo.h
	ftp.h
	local.h
	protocol.h
	smtp.h
	sockinet.h
	sockstream.h
	sockunix.h
	fork.h
	pipestream.h
	sig.h
	socketerr.h
	ossock.h
	DESTINATION ${header_destination}/socket++)
	
add_subdirectory(test)
