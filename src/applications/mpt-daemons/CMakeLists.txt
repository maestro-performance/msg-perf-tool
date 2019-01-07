if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	# Receiver files
	AddService(maestro-worker/config mpt-worker-daemon)
endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

include_directories(common)

add_library(
	mpt-daemon-common STATIC
    common/daemon_common.c
)


add_executable(
	mpt-worker-daemon
		maestro-worker/worker_main.c
		maestro-worker/worker_daemon.c
)

target_link_libraries(
	mpt-worker-daemon
	mpt-daemon-common
	mpt-common
	mpt-api-common
	mpt-orchestration-maestro
	mpt-worker
)

install(TARGETS
	mpt-worker-daemon
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
