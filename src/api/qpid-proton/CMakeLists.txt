add_definitions(-D_GNU_SOURCE -D_POSIX_C_SOURCE)

add_library(
	mpt-proton
        proton-context.c
        proton-wrapper.c
)

target_link_libraries(
    mpt-proton
    qpid-proton
)





