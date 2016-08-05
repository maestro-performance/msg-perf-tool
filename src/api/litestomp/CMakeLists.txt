add_definitions(-D_GNU_SOURCE -D_POSIX_C_SOURCE)

add_library(
	mpt-stomp SHARED
        stomp-context.c
        stomp-wrapper.c
)

target_link_libraries(
    mpt-stomp
    litestomp
)

install(TARGETS mpt-stomp
	LIBRARY DESTINATION ${CPP_LIBRARY_DIR}
)
