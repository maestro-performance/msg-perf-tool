if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	# Receiver files
	AddService(receiver/config mpt-receiver-daemon)
	AddService(sender/config mpt-sender-daemon)
endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

include_directories(common)

add_library(
	mpt-daemon-common STATIC
    common/daemon_common.c
)

add_executable(
	mpt-receiver-daemon
	receiver/receiverd_main.c
	receiver/receiverd_worker.c
)

target_link_libraries(
	mpt-receiver-daemon
	mpt-daemon-common
	mpt-common
	mpt-api-common
	mpt-orchestration-maestro
	mpt-worker
)

install(TARGETS
	mpt-receiver-daemon
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
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
