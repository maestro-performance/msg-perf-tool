if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	# Receiver files
	configure_file(receiver/config/mpt-receiver-daemon.service.in
		${CMAKE_BUILD_SYSTEMD_UNIT_PATH}/mpt-receiver-daemon.service
		@ONLY
	)

	configure_file(receiver/config/mpt-receiver-daemon.in
		${CMAKE_BUILD_SYSCONFIG_PATH}/mpt-receiver-daemon
		@ONLY
	)

	install(FILES
		${CMAKE_BUILD_SYSTEMD_UNIT_PATH}/mpt-receiver-daemon.service
		DESTINATION ${CMAKE_INSTALL_SYSTEMD_UNIT_PATH}
	)

	install(FILES
		${CMAKE_BUILD_SYSCONFIG_PATH}/mpt-receiver-daemon
		DESTINATION ${CMAKE_INSTALL_SYSCONFIG_PATH}
	)


	# Sender files
	configure_file(sender/config/mpt-sender-daemon.service.in
		${CMAKE_BUILD_SYSTEMD_UNIT_PATH}/mpt-sender-daemon.service
		@ONLY
	)


	configure_file(sender/config/mpt-sender-daemon.in
		${CMAKE_BUILD_SYSCONFIG_PATH}/mpt-sender-daemon
		@ONLY
	)

	install(FILES
		${CMAKE_BUILD_SYSTEMD_UNIT_PATH}/mpt-sender-daemon.service
		DESTINATION ${CMAKE_INSTALL_SYSTEMD_UNIT_PATH}
	)

	install(FILES
		${CMAKE_BUILD_SYSCONFIG_PATH}/mpt-sender-daemon
		DESTINATION ${CMAKE_INSTALL_SYSCONFIG_PATH}
	)

	# Inspector files
	configure_file(broker/config/mpt-broker-inspector.service.in
		${CMAKE_BUILD_SYSTEMD_UNIT_PATH}/mpt-broker-inspector.service
		@ONLY
	)


	configure_file(broker/config/mpt-broker-inspector.in
		${CMAKE_BUILD_SYSCONFIG_PATH}/mpt-broker-inspector
		@ONLY
	)

	install(FILES
		${CMAKE_BUILD_SYSTEMD_UNIT_PATH}/mpt-broker-inspector.service
		DESTINATION ${CMAKE_INSTALL_SYSTEMD_UNIT_PATH}
	)

	install(FILES
		${CMAKE_BUILD_SYSCONFIG_PATH}/mpt-broker-inspector
		DESTINATION ${CMAKE_INSTALL_SYSCONFIG_PATH}
	)

endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")


add_executable(
	mpt-receiver-daemon
	receiver/receiverd_main.c
	receiver/receiverd_worker.c
)

target_link_libraries(
	mpt-receiver-daemon
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
	mpt-common
	mpt-api-common
	mpt-orchestration-maestro
	mpt-worker
)

install(TARGETS
	mpt-broker-inspector
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)