if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	add_definitions(-DLINUX_BUILD -D_GNU_SOURCE)
else (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	message(STATUS "Compiling for " ${CMAKE_SYSTEM_NAME} "")

	if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
		set(CMAKE_MACOSX_RPATH TRUE)
		add_definitions(-D__OSX__)
	endif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/target/${CMAKE_INSTALL_BINDIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/target/${CMAKE_INSTALL_LIBDIR})
set (CTEST_BINARY_DIRECTORY ${CMAKE_BINARY_DIR}/target/tests/${CMAKE_INSTALL_BINDIR})

find_library(MATH_LIB NAMES m)
message(STATUS "Math library found at ${MATH_LIB}")

# GRU
find_path(GRU_INCLUDE_DIR common/gru_base.h
        PATH_SUFFIXES gru-0
				HINTS ${GRU_DIR}/${CMAKE_INSTALL_INCLUDEDIR})
find_library(GRU_LIB NAMES gru-0
				HINTS ${GRU_DIR}/${CMAKE_INSTALL_LIBDIR})

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
				PATH_SUFFIXES bmic-0
				HINTS ${BMIC_DIR}/${CMAKE_INSTALL_INCLUDEDIR})
find_library(BMIC_BASE_LIB NAMES bmic-base-0
				HINTS ${BMIC_DIR}/${CMAKE_INSTALL_LIBDIR})
find_library(BMIC_MANAGEMENT_LIB NAMES bmic-management-0
				HINTS ${BMIC_DIR}/${CMAKE_INSTALL_LIBDIR})
find_library(BMIC_PRODUCT_LIB NAMES bmic-product-0
				HINTS ${BMIC_DIR}/${CMAKE_INSTALL_LIBDIR})

message(STATUS "BMIC headers found at ${BMIC_INCLUDE_DIR}")
message(STATUS "BMIC base library found at ${BMIC_BASE_LIB}")
message(STATUS "BMIC management library found at ${BMIC_MANAGEMENT_LIB}")
message(STATUS "BMIC product library found at ${BMIC_PRODUCT_LIB}")

find_path(URIPARSER_INCLUDE_DIR uriparser/Uri.h)
find_library(URIPARSER_LIB NAMES uriparser liburiparser)

message(STATUS "URIParser found on ${URIPARSER_INCLUDE_DIR}")
message(STATUS "URIParser library found at ${URIPARSER_LIB}")

