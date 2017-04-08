add_executable(
    mpt-sender
    sender_main.c
    sender_perf_main.c
    sender_tune_main.c
    sender_worker.c
    tune_worker.c
)

target_link_libraries(
	mpt-sender
	mpt-common
        mpt-api-common
)

install(TARGETS
        mpt-sender
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
