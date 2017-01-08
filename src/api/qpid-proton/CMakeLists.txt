add_definitions(-D_GNU_SOURCE -D_POSIX_C_SOURCE)

find_path(PROTON_INCLUDE_DIR proton/messenger.h)
find_library(PROTON_LIB NAMES qpid-proton)

message(STATUS "QPid Proton headers found at ${PROTON_INCLUDE_DIR}")
message(STATUS "QPid Proton library found at ${PROTON_LIB}")

include_directories(${PROTON_LIB})

add_library(
	mpt-proton SHARED
        proton-context.c
        proton-wrapper.c
)

target_link_libraries(
    mpt-proton
    qpid-proton
)

install(TARGETS mpt-proton
	LIBRARY DESTINATION ${CPP_LIBRARY_DIR}
)
