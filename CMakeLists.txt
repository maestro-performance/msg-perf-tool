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


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/target/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/target/lib)

set(RUNTIME_DIR "bin")
set(CPP_INCLUDE_DIR "include")
set(SHARED_DIR "share")
set(MPT_SHARED_DIR ${SHARED_DIR}/mpt)

SET(AMQP_SUPPORT ON CACHE BOOL "Enable AMQP 1.0 support")
SET(STOMP_SUPPORT ON CACHE BOOL "Enable STOMP 1.2 support")


if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_definitions(-DLINUX_BUILD)

    if(EXISTS "/etc/redhat-release")
        add_definitions(-D__RH_DISTRO__)
    endif(EXISTS "/etc/redhat-release")

    if(EXISTS "/etc/debian_version")
        add_definitions(-D__DEBIAN_DISTRO__)
    endif(EXISTS "/etc/debian_version")

    if(EXISTS "/etc/ubuntu-release")
        add_definitions(-D__UBUNTU_DISTRO__)
    endif(EXISTS "/etc/ubuntu-release")
endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

#### GNU Compiler flags

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_C_FLAGS "-Wall -Wshadow -std=c99 ${CMAKE_C_FLAGS}" CACHE STRING
		"Flags used by the compiler during all build types." FORCE
	)
        set(CMAKE_C_FLAGS_DEBUG "-fdiagnostics-color=auto -g ${CMAKE_C_FLAGS_DEBUG}" CACHE STRING
		"Flags used by the compiler during debug build." FORCE
	)
        set(CMAKE_C_FLAGS_RELEASE "-O2 -fomit-frame-pointer -D_FORTIFY_SOURCE=1 ${CMAKE_C_FLAGS_RELEASE}" CACHE STRING
		"Flags used by the compiler during release build." FORCE
	)
endif (CMAKE_COMPILER_IS_GNUCXX)


include_directories(src/common)
add_subdirectory(src/common objs/common)

#### PROTON STUFF

if (${AMQP_SUPPORT})
    message(STATUS "AMQP support is enabled")
    add_definitions(-D__AMQP_SUPPORT__)
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
endif(${AMQP_SUPPORT})

if (${STOMP_SUPPORT})
    message(STATUS "Stomp support is enabled")
    add_definitions(-D__STOMP_SUPPORT__)
    set(STOMP_DIR /usr CACHE String "Litestomp base directory")

    include_directories(
            ${STOMP_DIR}/include
            .
    )

    if (${APP_BUILD_PLATFORM} EQUAL "64")
            link_directories(
                    ${STOMP_DIR}/lib64
            )
    else(${APP_BUILD_PLATFORM} EQUAL "64")
            link_directories(
                    ${STOMP_DIR}/lib
            )
    endif(${APP_BUILD_PLATFORM} EQUAL "64")

    include_directories(src/api/litestomp)
    add_subdirectory(src/api/litestomp objs/api/litestomp)
endif(${STOMP_SUPPORT})

add_subdirectory(src/receiver objs/receiver)
add_subdirectory(src/sender objs/sender)
add_subdirectory(src/runner objs/runner)
add_subdirectory(src/loader objs/loader)
