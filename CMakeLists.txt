project(mpt)
cmake_minimum_required(VERSION 2.8)

include(GNUInstallDirs)
include(CheckLists.cmake)
include(CMakeLists.default.cmake)

set(MPT_VERSION 0.1.1)
set(MPT_VERSION_MAJOR 0)
set(MPT_VERSION_MINOR 1)
set(MPT_VERSION_PATCH 1)

set(GRU_DIR "/usr" CACHE STRING "GRU directory")
set(BUILD_WITH_EXTRA_DEBUG OFF CACHE BOOL "Build extra debug information")
set(BUILD_WITH_UNIT_TESTS ON CACHE BOOL "Build unit tests")

add_definitions(-DPROJECT_NAME="${PROJECT_NAME}" -DMPT_VERSION="${MPT_VERSION}" -DMPT_PQUEUE)

if (UNIX)
	message(STATUS "Adding Unix related build rules")
	include(CMakeLists.unix.cmake)
else (UNIX)
	if (WIN32)
		message(STATUS "Adding Windows related build rules")
		include(CMakeLists.windows.cmake)
	endif (WIN32)
endif(UNIX)

set(MQTT_SUPPORT ON CACHE BOOL "Enable MQTT 3.x support")

include_directories(${GRU_INCLUDE_DIR}
    ${URIPARSER_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
    ${MSGPACK_INCLUDE_DIR}
    ${HDR_HISTOGRAM_C_INCLUDE_DIR})

link_libraries(${MATH_LIB}
    ${RT_LIB}
    ${GRU_LIB}
    ${URIPARSER_LIB}
    ${UUID_LIB}
    ${ZLIB_LIB}
    ${MSGPACK_LIB}
    ${HDR_HISTOGRAM_C_LIB}
)

include_directories(src/common)
add_subdirectory(src/common objs/common)


if (${MQTT_SUPPORT})
    message(STATUS "MQTT support is enabled")
    add_definitions(-D__MQTT_SUPPORT__)

    include_directories(src/api/paho-c)
    add_subdirectory(src/api/paho-c objs/api/paho-c)
endif(${MQTT_SUPPORT})

include_directories(src/api)
add_subdirectory(src/api objs/api)

include_directories(src/orchestration)
add_subdirectory(src/orchestration objs/orchestration)

add_subdirectory(src/worker objs/worker)
include_directories(src/worker)

add_subdirectory(src/applications/maestro objs/applications/maestro)
add_subdirectory(src/applications/mpt-daemons objs/applications/mpt-daemons)

add_custom_target(format
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/util/format.sh
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

if (BUILD_WITH_UNIT_TESTS)
	enable_testing()

	include_directories(test)
	add_subdirectory(test)
endif (BUILD_WITH_UNIT_TESTS)
