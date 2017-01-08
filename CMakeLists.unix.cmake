if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	add_definitions(-DLINUX_BUILD -D_GNU_SOURCE)
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		message(STATUS "64 bits compiler detected")
		set(APP_BUILD_PLATFORM 64)
		set(APP_BUILD_PLATFORM_NAME "x86_64")
		set(CPP_LIBRARY_DIR "lib64")
	else (CMAKE_SIZEOF_VOID_P EQUAL 8)
		message(STATUS "32 bits compiler detected")
		set(APP_BUILD_PLATFORM 32)
		set(APP_BUILD_PLATFORM_NAME "i686")
		set(CPP_LIBRARY_DIR "lib")
	endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

	if(EXISTS "/etc/debian_version")
			set(CPP_LIBRARY_DIR "lib")
	endif(EXISTS "/etc/debian_version")

else (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	message(STATUS "Compiling for " ${CMAKE_SYSTEM_NAME} "")
	set(CPP_LIBRARY_DIR "lib")

	if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
		set(CMAKE_MACOSX_RPATH TRUE)
		add_definitions(-D__OSX__)
	endif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/target/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/target/lib)
set (CTEST_BINARY_DIRECTORY ${CMAKE_BINARY_DIR}/target/tests/bin)


find_library(MATH_LIB NAMES m)
message(STATUS "Math library found at ${MATH_LIB}")

# GRU
find_path(GRU_INCLUDE_DIR common/gru_base.h
          PATH_SUFFIXES gru-0)
find_library(GRU_LIB NAMES gru-0)

message(STATUS "GRU headers found at ${GRU_INCLUDE_DIR}")
message(STATUS "GRU library found at ${GRU_LIB}")

find_path(JSON_INCLUDE_DIR json.h
          PATH_SUFFIXES json json-c)

find_library(JSON_LIB NAMES json json-c)
link_libraries(${JSON_LIB})

message(STATUS "JSON headers found at ${JSON_INCLUDE_DIR}")
message(STATUS "JSON library found at ${JSON_LIB}")

# BMIC
find_path(BMIC_INCLUDE_DIR base/common/bmic_object.h
					PATH_SUFFIXES bmic-0)
find_library(BMIC_BASE_LIB NAMES bmic-base-0)
find_library(BMIC_MANAGEMENT_LIB NAMES bmic-management-0)
find_library(BMIC_PRODUCT_LIB NAMES bmic-product-0)

message(STATUS "BMIC headers found at ${BMIC_INCLUDE_DIR}")
message(STATUS "BMIC base library found at ${BMIC_BASE_LIB}")
message(STATUS "BMIC management library found at ${BMIC_MANAGEMENT_LIB}")
message(STATUS "BMIC product library found at ${BMIC_PRODUCT_LIB}")
