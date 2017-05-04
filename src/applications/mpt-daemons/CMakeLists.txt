if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	# Receiver files
	configure_file(receiver/config/receiverd.service.in
		${CMAKE_BUILD_SYSTEMD_UNIT_PATH}/receiverd.service
		@ONLY
	)

	configure_file(receiver/config/receiverd.in
		${CMAKE_BUILD_SYSCONFIG_PATH}/receiverd
		@ONLY
	)

	install(FILES
		${CMAKE_BUILD_SYSTEMD_UNIT_PATH}/receiverd.service
		DESTINATION ${CMAKE_INSTALL_SYSTEMD_UNIT_PATH}
	)

	install(FILES
		${CMAKE_BUILD_SYSCONFIG_PATH}/receiverd
		DESTINATION ${CMAKE_INSTALL_SYSCONFIG_PATH}
	)


	# Sender files
	configure_file(sender/config/senderd.service.in
		${CMAKE_BUILD_SYSTEMD_UNIT_PATH}/senderd.service
		@ONLY
	)


	configure_file(sender/config/senderd.in
		${CMAKE_BUILD_SYSCONFIG_PATH}/senderd
		@ONLY
	)

	install(FILES
		${CMAKE_BUILD_SYSTEMD_UNIT_PATH}/senderd.service
		DESTINATION ${CMAKE_INSTALL_SYSTEMD_UNIT_PATH}
	)

	install(FILES
		${CMAKE_BUILD_SYSCONFIG_PATH}/senderd
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
