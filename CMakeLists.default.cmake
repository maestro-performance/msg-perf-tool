# uninstall
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/dist/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/dist/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/dist/cmake_uninstall.cmake)

set(RUNTIME_DIR "bin")
set(CPP_INCLUDE_DIR "include")
set(SHARED_DIR "share")

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING
      	"Choose the type of build, options are: Debug Release."
      	FORCE
	)
endif(NOT CMAKE_BUILD_TYPE)

if (CMAKE_COMPILER_IS_GNUCXX)
        if (BUILD_WITH_EXTRA_DEBUG)
			add_definitions(-DMPT_DEBUG=2)
	endif (BUILD_WITH_EXTRA_DEBUG)

	set(CMAKE_C_FLAGS "-Wall -Wshadow -Wconversion -Wno-sign-conversion -fstrict-aliasing -std=c99 ${CMAKE_USER_C_FLAGS}" CACHE STRING
		"Flags used by the compiler during all build types." FORCE
	)

	set(CMAKE_C_FLAGS_DEBUG "-fdiagnostics-color=auto -g -fstrict-aliasing ${CMAKE_USER_C_FLAGS}"  CACHE STRING
		"Flags used by the compiler during debug build." FORCE
	)

	set(CMAKE_C_FLAGS_RELEASE "-O2 -fomit-frame-pointer -D_FORTIFY_SOURCE=1 -fstrict-aliasing ${CMAKE_USER_C_FLAGS}" CACHE STRING
		"Flags used by the compiler during release build." FORCE
	)
endif (CMAKE_COMPILER_IS_GNUCXX)
