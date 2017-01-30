set(LITESTOMP_DIR "/usr" CACHE STRING "Litestomp directory")

find_path(LITESTOMP_INCLUDE_DIR litestomp-0/stomp_types.h
		HINTS ${LITESTOMP_DIR}/${CMAKE_INSTALL_INCLUDEDIR})
find_library(LITESTOMP_LIB NAMES litestomp-0
		HINTS ${LITESTOMP_DIR}/${CMAKE_INSTALL_LIBDIR})

message(STATUS "LiteSTOMP hint ${LITESTOMP_DIR}")
message(STATUS "LiteSTOMP headers found at ${LITESTOMP_INCLUDE_DIR}")
message(STATUS "LiteSTOMP library found at ${LITESTOMP_LIB}")


find_path(APR_INCLUDE_DIR apr_general.h
        PATH_SUFFIXES apr-1 apr-1.0)
find_library(APR_LIB NAMES apr-1)

message(STATUS "APR headers found at ${APR_INCLUDE_DIR}")
message(STATUS "APR library found at ${APR_LIB}")

find_path(APR_UTIL_INCLUDE_DIR apu.h
        PATH_SUFFIXES apr-1 apr-1.0)
find_library(APR_UTIL_LIB NAMES aprutil-1)

message(STATUS "APR util headers found at ${APR_UTIL_INCLUDE_DIR}")
message(STATUS "APR util library found at ${APR_UTIL_LIB}")

include_directories(${LITESTOMP_INCLUDE_DIR} ${APR_UTIL_INCLUDE_DIR})

add_library(
	mpt-stomp SHARED
	stomp-context.c
	stomp-wrapper.c
)

target_link_libraries(
    mpt-stomp
	mpt-common
    ${LITESTOMP_LIB}
	${APR_UTIL_LIB}
)

install(TARGETS mpt-stomp
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
