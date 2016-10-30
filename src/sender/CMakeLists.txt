add_definitions(-D_GNU_SOURCE -D_POSIX_C_SOURCE)

if (${CMAKE_SYSTEM_NAME} EQUAL "Linux")
 add_definitions(-DLINUX_BUILD)
endif (${CMAKE_SYSTEM_NAME} EQUAL "Linux")

add_executable(
    mpt-sender
    sender_main.c
    sender_perf_main.c
    sender_tune_main.c
    message_sender.c
    tune_sender.c
)

target_link_libraries(
	mpt-sender
	mpt-common
        mpt-api-common
)

install(TARGETS
        mpt-sender
	RUNTIME DESTINATION ${RUNTIME_DIR}
)
