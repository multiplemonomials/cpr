add_library(socket++
	echo.cpp
	ftp.cpp
	protocol.cpp
	smtp.cpp
	sockinet.cpp
	sockstream.cpp
	sockunix.cpp)

install (TARGETS socket++ DESTINATION lib)

include_directories("${PROJECT_SOURCE_DIR}/cpr/socket++")

install (FILES 
	echo.h
	ftp.h
	local.h
	protocol.h
	smtp.h
	sockinet.h
	sockstream.h
	sockunix.h
	DESTINATION ${header_destination}/socket++)
	
add_subdirectory(test)
