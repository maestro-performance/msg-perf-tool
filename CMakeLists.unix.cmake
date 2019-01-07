if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	add_definitions(-DLINUX_BUILD -D_GNU_SOURCE)

	find_library(RT_LIB NAMES rt)
	message(STATUS "RT library found at ${RT_LIB}")

	SET(SYSTEMD_SUPPORT ON CACHE BOOL "Enable systemd support")
	# Fixed directory for service files
	set(SERVICE_INSTALL_PREFIX "" CACHE STRING "Install prefix for service files (for packaging only)")

	set(CMAKE_INSTALL_SYSTEMD_UNIT_PATH ${SERVICE_INSTALL_PREFIX}/usr/lib/systemd/system)
	set(CMAKE_BUILD_SYSTEMD_UNIT_PATH ${CMAKE_BINARY_DIR}/target/${CMAKE_INSTALL_SYSTEMD_UNIT_PATH})

	# Fixed directory
	set(CMAKE_INSTALL_SYSCONFIG_PATH ${SERVICE_INSTALL_PREFIX}/etc/sysconfig)
	set(CMAKE_BUILD_SYSCONFIG_PATH ${CMAKE_BINARY_DIR}/target/${CMAKE_INSTALL_SYSCONFIG_PATH})

else (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	message(STATUS "Compiling for " ${CMAKE_SYSTEM_NAME} "")

	set(RT_LIB "")

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

find_path(URIPARSER_INCLUDE_DIR uriparser/Uri.h)
find_library(URIPARSER_LIB NAMES uriparser liburiparser)

message(STATUS "URIParser found on ${URIPARSER_INCLUDE_DIR}")
message(STATUS "URIParser library found at ${URIPARSER_LIB}")

find_path(UUID_INCLUDE_DIR uuid/uuid.h)
find_library(UUID_LIB NAMES uuid libuuid)

message(STATUS "UUID headers found on ${UUID_INCLUDE_DIR}")
message(STATUS "UUID library found at ${UUID_LIB}")

find_path(ZLIB_INCLUDE_DIR zlib.h)
find_library(ZLIB_LIB NAMES z libz)

message(STATUS "zlib headers found on ${ZLIB_INCLUDE_DIR}")
message(STATUS "zlib library found at ${ZLIB_LIB}")

find_path(MSGPACK_INCLUDE_DIR msgpack.h)
find_library(MSGPACK_LIB NAMES msgpackc libmsgpackc)

message(STATUS "MessagePack headers found on ${MSGPACK_INCLUDE_DIR}")
message(STATUS "MessagePack library found at ${MSGPACK_LIB}")


find_path(HDR_HISTOGRAM_C_INCLUDE_DIR hdr/hdr_histogram.h)
find_library(HDR_HISTOGRAM_C_LIB NAMES hdr_histogram libhdr_histogram)

message(STATUS "HDR Histogram C headers found on ${HDR_HISTOGRAM_C_INCLUDE_DIR}")
message(STATUS "HDR Histogram C library found at ${HDR_HISTOGRAM_C_LIB}")

# Installs service configuration files (ie.: for systemd daemons). For systemd daemons
# It requires 2 files: a <service_name>.in file, containing the service startup
# configuration and a <service_name.service.in, which is a systemd-compliant service
# file.
macro(AddService SERVICE_CONFIG_SOURCE SERVICE_NAME)
	if (${SYSTEMD_SUPPORT})
		configure_file(${SERVICE_CONFIG_SOURCE}/${SERVICE_NAME}.service.in
				${CMAKE_BUILD_SYSTEMD_UNIT_PATH}/${SERVICE_NAME}.service
				@ONLY
				)

		configure_file(${SERVICE_CONFIG_SOURCE}/${SERVICE_NAME}.in
				${CMAKE_BUILD_SYSCONFIG_PATH}/${SERVICE_NAME}
				@ONLY
				)

		install(FILES
				${CMAKE_BUILD_SYSTEMD_UNIT_PATH}/${SERVICE_NAME}.service
				DESTINATION ${CMAKE_INSTALL_SYSTEMD_UNIT_PATH}
				)

		install(FILES
				${CMAKE_BUILD_SYSCONFIG_PATH}/${SERVICE_NAME}
				DESTINATION ${CMAKE_INSTALL_SYSCONFIG_PATH}
				)
	endif (${SYSTEMD_SUPPORT})
endmacro(AddService)