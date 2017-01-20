add_definitions(-D_GNU_SOURCE -D_POSIX_C_SOURCE)

set(PAHOC_DIR "/usr" CACHE STRING "Proton directory")

find_path(PAHOC_INCLUDE_DIR proton/messenger.h
	HINTS ${PAHOC_DIR}/include)
find_library(PAHOC_LIB NAMES paho-mqtt3c
HINTS ${PAHOC_DIR}/lib ${PROTON_DIR}/lib64)

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
    ${PAHOC_LIB}
)

install(TARGETS mpt-paho
	LIBRARY DESTINATION ${CPP_LIBRARY_DIR}
)
