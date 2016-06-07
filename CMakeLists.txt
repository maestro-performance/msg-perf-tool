project(mpt)
cmake_minimum_required(VERSION 2.8)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/target/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/target/lib)

set(RUNTIME_DIR "bin")
set(CPP_LIBRARY_DIR "lib")
set(CPP_INCLUDE_DIR "include")


if (${CMAKE_SYSTEM_NAME} EQUAL "Linux") 
 add_definitions(-DLINUX_BUILD) 
endif (${CMAKE_SYSTEM_NAME} EQUAL "Linux")

if( CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(STATUS "64 bits compiler detected")
    set(APP_BUILD_PLATFORM 64)
    set(APP_BUILD_PLATFORM_NAME "x64")
else(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(STATUS "32 bits compiler detected")
    set(APP_BUILD_PLATFORM 32)
    set(APP_BUILD_PLATFORM_NAME "x84")
endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

#### GNU Compiler flags

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_C_FLAGS "-Wall -Wshadow -fdiagnostics-color=auto -g -std=c99" CACHE STRING
		"Flags used by the compiler during all build types." FORCE
	)
endif (CMAKE_COMPILER_IS_GNUCXX)


include_directories(src/common)
add_subdirectory(src/common objs/common)

#### PROTON STUFF

set(PROTON_DIR /usr CACHE String "QPID Proton base directory")

include_directories(
	${PROTON_DIR}/include
        .

)

if (${APP_BUILD_PLATFORM} EQUAL "64")
	link_directories(
		${PROTON_DIR}/lib64
	)
else(${APP_BUILD_PLATFORM} EQUAL "64")
	link_directories(
		${PROTON_DIR}/lib
	)
endif(${APP_BUILD_PLATFORM} EQUAL "64")

include_directories(src/api/qpid-proton)
add_subdirectory(src/api/qpid-proton objs/api/qpid-proton)

add_subdirectory(src/receiver objs/receiver)
add_subdirectory(src/sender objs/sender)
add_subdirectory(src/plot objs/plot)