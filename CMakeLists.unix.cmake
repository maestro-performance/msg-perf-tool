if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	add_definitions(-DLINUX_BUILD -D_GNU_SOURCE)

	find_library(RT_LIB NAMES rt)
	message(STATUS "RT library found at ${RT_LIB}")

	SET(SYSTEMD_SUPPORT ON CACHE BOOL "Enable systemd support")
	# Fixed directory for service files
	set(SERVICE_INSTALL_PREFIX "/" CACHE STRING "Install prefix for service files (for packaging only)")

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

# Installs service configuration files (ie.: for systemd daemons). For systemd daemons
# It requires 2 files: a <service_name>.in file, containing the service startup
# configuration and a <service_name.service.in, which is a systemd-compliant service
# file.
macro(AddService SERVICE_CONFIG_SOURCE SERVICE_NAME)
	if (NOT ${SYSTEMD_SUPPORT})
		configure_file(${SERVICE_CONFIG_SOURCE}/${SERVICE_NAME}.legacy.sh.in
			${CMAKE_BINARY_DIR}/etc/init.d/${SERVICE_NAME}/${SERVICE_NAME}
			@ONLY
		)

		configure_file(${SERVICE_CONFIG_SOURCE}/${SERVICE_NAME}.in
			${CMAKE_BUILD_SYSCONFIG_PATH}/${SERVICE_NAME}
			@ONLY
		)

		install(FILES
			${CMAKE_BINARY_DIR}/etc/init.d/${SERVICE_NAME}/${SERVICE_NAME}
			DESTINATION ${SERVICE_INSTALL_PREFIX}/etc/init.d/
			PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
		)

		install(FILES
			${CMAKE_BUILD_SYSCONFIG_PATH}/${SERVICE_NAME}
			DESTINATION ${CMAKE_INSTALL_SYSCONFIG_PATH}
		)

	else (NOT ${SYSTEMD_SUPPORT})
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
	endif (NOT ${SYSTEMD_SUPPORT})
endmacro(AddService)