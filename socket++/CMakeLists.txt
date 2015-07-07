add_library(socket++
	echo.cpp
	ftp.cpp
	protocol.cpp
	smtp.cpp
	sockinet.cpp
	sockstream.cpp
	sockunix.cpp
	fork.cpp
	#pipestream.cpp
	processstream.cpp
	sig.cpp
	socketerr.cpp
	filelikebuf.cpp)

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
	filelikebuf.h
	processtream.h
	DESTINATION ${header_destination}/socket++)
	
add_subdirectory(test)
