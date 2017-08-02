if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	# Receiver files
	AddService(receiver/config mpt-receiver-daemon)
	AddService(sender/config mpt-sender-daemon)
	AddService(broker/config mpt-broker-inspector)
	AddService(data-server/config mpt-data-server)
endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

configure_file(data-server/mpt-data-server.sh.in
	${CMAKE_BINARY_DIR}/target/${CMAKE_INSTALL_BINDIR}/mpt-data-server
	@ONLY
)

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


add_executable(
	mpt-broker-inspector
	broker/brokerd_main.c
	broker/brokerd_worker.c
)

target_link_libraries(
	mpt-broker-inspector
	mpt-daemon-common
	mpt-common
	mpt-api-common
	mpt-orchestration-maestro
	mpt-worker
)

install(TARGETS
	mpt-broker-inspector
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)