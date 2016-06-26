add_definitions(-D_GNU_SOURCE -D_POSIX_C_SOURCE)

add_library(
	mpt-stomp
        stomp-context.c
        stomp-wrapper.c
)

target_link_libraries(
    mpt-stomp
    litestomp
)






