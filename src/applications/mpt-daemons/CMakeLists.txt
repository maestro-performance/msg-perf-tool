if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	# Receiver files
	AddService(sender/config mpt-sender-daemon)
endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

include_directories(common)

add_library(
	mpt-daemon-common STATIC
    common/daemon_common.c
)


add_executable(
	mpt-sender-daemon
	sender/senderd_main.c
	sender/senderd_worker.c
)

target_link_libraries(
	mpt-sender-daemon
	mpt-daemon-common
	mpt-common
	mpt-api-common
	mpt-orchestration-maestro
	mpt-worker
)

install(TARGETS
	mpt-sender-daemon
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
