cmake_minimum_required (VERSION 2.6)

#hush, annoying warnings
if(POLICY CMP0017)
 cmake_policy(SET CMP0017 NEW)
endif(POLICY CMP0017)

project (CPR)

enable_testing()

include (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
include (${CMAKE_ROOT}/Modules/CheckIncludeFileCXX.cmake)
include (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)

# The version number.
set (CPR_VERSION_MAJOR 1)
set (CPR_VERSION_MINOR 0)

#leaving this in for now while code is in development
#enables debug flags
SET(CMAKE_BUILD_TYPE relwithdebinfo)

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} --std=gnu++11 -Wall -pthread")

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} --std=gnu++11 -Wall -g -pthread")

check_include_file_cxx(sstream HAVE_SSTREAM)
check_include_file_cxx("string" HAVE_STRING)

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
  "${PROJECT_SOURCE_DIR}/cpr/Config.h.in"
  "${PROJECT_SOURCE_DIR}/cpr/Config.h"
  )

set(header_destination "include/cpr") 
 
include_directories("${PROJECT_SOURCE_DIR}/cpr")

add_subdirectory ("cpr")
 
