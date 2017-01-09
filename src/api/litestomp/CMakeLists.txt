add_definitions(-D_GNU_SOURCE -D_POSIX_C_SOURCE)


find_path(LITESTOMP_INCLUDE_DIR stomp_types.h
					PATH_SUFFIXES litestomp-0)
find_library(LITESTOMP_LIB NAMES litestomp-0)

message(STATUS "LiteSTOMP headers found at ${LITESTOMP_INCLUDE_DIR}")
message(STATUS "LiteSTOMP library found at ${LITESTOMP_LIB}")

include_directories(${LITESTOMP_LIB})

add_library(
	mpt-stomp SHARED
        stomp-context.c
        stomp-wrapper.c
)

target_link_libraries(
    mpt-stomp
    litestomp-0
)

install(TARGETS mpt-stomp
	LIBRARY DESTINATION ${CPP_LIBRARY_DIR}
)
