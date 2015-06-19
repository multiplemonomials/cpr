cmake_minimum_required (VERSION 2.8.4)

#hush, annoying warnings
if(POLICY CMP0017)
 cmake_policy(SET CMP0017 NEW)
endif(POLICY CMP0017)

project(SOCKET++)

enable_testing()

include (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
include (${CMAKE_ROOT}/Modules/CheckIncludeFileCXX.cmake)
include (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)

# The version number.
set(SOCKET++_VERSION_MAJOR 2)
set(SOCKET++_VERSION_MINOR 0)

#leaving this in for now while code is in development
#enables debug flags
SET(CMAKE_BUILD_TYPE relwithdebinfo)

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} --std=c++11 -Wall -pthread -Wno-unknown-pragmas -iquote ${PROJECT_SOURCE_DIR}/socket++")

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g")

option(USE_EXTERN_C "Wrap C include files in extern C blocks.  Most system headers already have these blocks, but try it if you get link errors.")
option(USE_WINSOCK "Use the Windows WSA socket library as opposed to the UNIX one." ${WIN32})


if(USE_EXTERN_C)
	set(EXTERN_C_BEGIN "extern \"C\" {")
	set(EXTERN_C_END "}")
endif(USE_EXTERN_C)
	

check_include_file_cxx(sstream HAVE_SSTREAM)
check_include_file_cxx("string" HAVE_STRING)
check_include_file_cxx("memory" HAVE_MEMORY)


check_include_file(dflcn.h HAVE_DLFCN_H)
check_include_file(inttypes.h HAVE_INTTYPES_H)
check_include_file(memory.h HAVE_MEMORY_H)
check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(stdlib.h HAVE_STDLIB_H)
check_include_file(strings.h HAVE_STRINGS_H)
check_include_file(string.h HAVE_STRING_H)
check_include_file(sys/stat.h HAVE_SYS_STAT_H)
check_include_file(sys/stat.h HAVE_SYS_TYPES_H)
check_include_file(unistd.h HAVE_UNISTD_H)
check_include_file(sys/wait.h HAVE_SYS_WAIT_H)

check_function_exists(fork HAVE_FORK)
check_function_exists(vfork HAVE_VFORK)
check_function_exists(sys_siglist HAVE_SYS_SIGLIST)
check_function_exists(select HAVE_SELECT)
check_function_exists(sendmsg HAVE_SENDMSG)
check_function_exists(sigaction HAVE_SIGACTION)

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	add_custom_target(doc
		${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif(DOXYGEN_FOUND)

 
# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/socketpp-config.h.in"
  "${PROJECT_BINARY_DIR}/config/socketpp-config.h"
  )

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_BINARY_DIR}/config")
	
#install config header
install (FILES 
	${PROJECT_BINARY_DIR}/config/socketpp-config.h
	DESTINATION ${header_destination}/socket++)
	
set(header_destination "include/socket++") 
 
add_subdirectory("socket++")
 
