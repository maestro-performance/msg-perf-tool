add_executable(
    mpt-sender
    sender_main.c
    perf/perf_main.c
    perf/perf_worker.c
    tune/tune_main.c
    tune/tune_worker.c
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
