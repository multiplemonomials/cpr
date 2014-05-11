add_library(cprbase
	fork.cpp
	pipestream.cpp
	sig.cpp
	cprerr.cpp)

install (TARGETS cprbase DESTINATION lib)

include_directories("${PROJECT_SOURCE_DIR}/cpr/base")

install (FILES 
	fork.h
	pipestream.h
	sig.h
	cprerr.h
	DESTINATION ${header_destination}/base)
	
add_subdirectory(test)
