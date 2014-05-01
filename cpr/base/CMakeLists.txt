add_library(cprbase
	fork.cpp
	pipestream.cpp
	sig.cpp
	cprerr.cpp)

install (TARGETS cprbase DESTINATION lib)

include_directories("${PROJECT_SOURCE_DIR}/cpr/cprbase")

install (FILES 
	fork.h
	pipestream.h
	sig.h
	cprerr.h
	DESTINATION ${header_destination}/cprbase)
	
#add_subdirectory(test)
