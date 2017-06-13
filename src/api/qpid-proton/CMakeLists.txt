set(PROTON_DIR "/usr" CACHE STRING "Proton directory")

find_path(PROTON_INCLUDE_DIR proton/messenger.h
	HINTS ${PROTON_DIR}/${CMAKE_INSTALL_INCLUDEDIR})
find_library(PROTON_LIB NAMES qpid-proton
	HINTS ${PROTON_DIR}/${CMAKE_INSTALL_LIBDIR})

message(STATUS "QPid Proton hint ${PROTON_DIR}")
message(STATUS "QPid Proton headers found at ${PROTON_INCLUDE_DIR}")
message(STATUS "QPid Proton library found at ${PROTON_LIB}")

include_directories(${PROTON_INCLUDE_DIR})

add_library(
	mpt-proton SHARED
	proton-context.c
	proton-wrapper.c
	proton-handlers.c
)

target_link_libraries(
    mpt-proton
	mpt-common
    ${PROTON_LIB}
)

install(TARGETS mpt-proton
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
