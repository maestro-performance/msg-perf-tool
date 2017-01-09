add_definitions(-D_GNU_SOURCE -D_POSIX_C_SOURCE)

set(PROTON_DIR "/usr" CACHE STRING "Proton directory")

find_path(PROTON_INCLUDE_DIR proton/messenger.h
	HINTS ${PROTON_DIR}/include)
find_library(PROTON_LIB NAMES qpid-proton
	HINTS ${PROTON_DIR}/lib ${PROTON_DIR}/lib64)

message(STATUS "QPid Proton hint ${PROTON_DIR}")
message(STATUS "QPid Proton headers found at ${PROTON_INCLUDE_DIR}")
message(STATUS "QPid Proton library found at ${PROTON_LIB}")

include_directories(${PROTON_INCLUDE_DIR})

add_library(
	mpt-proton SHARED
        proton-context.c
        proton-wrapper.c
)

target_link_libraries(
    mpt-proton
    ${PROTON_LIB}
)

install(TARGETS mpt-proton
	LIBRARY DESTINATION ${CPP_LIBRARY_DIR}
)
