add_definitions(-D_GNU_SOURCE -D_POSIX_C_SOURCE)

set(PAHOC_DIR "/usr" CACHE STRING "Proton directory")

find_path(PAHOC_INCLUDE_DIR proton/messenger.h
	HINTS ${PAHOC_DIR}/include)
find_library(PAHOC_LIB NAMES paho-mqtt3c
HINTS ${PAHOC_DIR}/lib ${PROTON_DIR}/lib64)

message(STATUS "QPid Proton hint ${PROTON_DIR}")
message(STATUS "QPid Proton headers found at ${PROTON_INCLUDE_DIR}")
message(STATUS "QPid Proton library found at ${PROTON_LIB}")

include_directories(${PAHOC_LIB})

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
