add_definitions(-D_GNU_SOURCE -D_POSIX_C_SOURCE)

add_library(
	mpt-paho SHARED
        paho-context.c
        paho-wrapper.c
)

target_link_libraries(
    mpt-paho
    paho-mqtt3c
)

install(TARGETS mpt-paho
	LIBRARY DESTINATION ${CPP_LIBRARY_DIR}
)
