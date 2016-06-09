project(mpt)
cmake_minimum_required(VERSION 2.8)

if( CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(STATUS "64 bits compiler detected")
    set(APP_BUILD_PLATFORM 64)
    set(APP_BUILD_PLATFORM_NAME "x86_64")
    set(CPP_LIBRARY_DIR "lib64")
else(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(STATUS "32 bits compiler detected")
    set(APP_BUILD_PLATFORM 32)
    set(APP_BUILD_PLATFORM_NAME "i686")
    set(CPP_LIBRARY_DIR "lib")
endif(CMAKE_SIZEOF_VOID_P EQUAL 8)


set (CPACK_RPM_PACKAGE_SUMMARY "Messaging Performance Tool")
set (CPACK_RPM_PACKAGE_NAME "msg-perf-tool")
set (CPACK_RPM_PACKAGE_VERSION "0.0.1")
set (CPACK_RPM_PACKAGE_RELEASE "1")
set (CPACK_RPM_PACKAGE_LICENSE "Apache v2")
set (CPACK_RPM_PACKAGE_GROUP "Development/Tools")
set (CPACK_RPM_PACKAGE_VENDOR "none")
set (CPACK_RPM_PACKAGE_DESCRIPTION "A tool for measuring messaging system performance")
set (CPACK_RPM_PACKAGE_ARCHITECTURE ${APP_BUILD_PLATFORM_NAME})

INCLUDE(CPack)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/target/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/target/lib)

set(RUNTIME_DIR "bin")
set(CPP_INCLUDE_DIR "include")
set(SHARED_DIR "share")
set(MPT_SHARED_DIR ${SHARED_DIR}/mpt)


if (${CMAKE_SYSTEM_NAME} EQUAL "Linux")
 add_definitions(-DLINUX_BUILD)
endif (${CMAKE_SYSTEM_NAME} EQUAL "Linux")

#### GNU Compiler flags

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_C_FLAGS "-Wall -Wshadow -std=c99" CACHE STRING
		"Flags used by the compiler during all build types." FORCE
	)
        set(CMAKE_C_FLAGS_DEBUG "-fdiagnostics-color=auto -g" CACHE STRING
		"Flags used by the compiler during debug build." FORCE
	)
        set(CMAKE_C_FLAGS_RELEASE "-O2 -fomit-frame-pointer" CACHE STRING
		"Flags used by the compiler during release build." FORCE
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
