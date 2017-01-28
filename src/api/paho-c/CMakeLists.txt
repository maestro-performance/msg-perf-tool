add_definitions(-D_GNU_SOURCE -D_POSIX_C_SOURCE)

set(PAHOC_DIR "/usr" CACHE STRING "Proton directory")

find_path(PAHOC_INCLUDE_DIR MQTTClient.h
	HINTS ${PAHOC_DIR}/${CMAKE_INSTALL_INCLUDEDIR})
find_library(PAHOC_LIB NAMES paho-mqtt3c
    HINTS ${PAHOC_DIR}/${CMAKE_INSTALL_LIBDIR})

message(STATUS "Paho C hint ${PAHOC_DIR}")
message(STATUS "Paho C headers found at ${PAHOC_DIR_INCLUDE_DIR}")
message(STATUS "Paho C library found at ${PAHOC_DIR_LIB}")

include_directories(${PAHOC_INCLUDE_DIR})

add_library(
    mpt-paho SHARED
    paho-context.c
    paho-wrapper.c
)

target_link_libraries(
    mpt-paho
    mpt-common
    ${PAHOC_LIB}
)

install(TARGETS mpt-paho
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
