add_executable(
	mpt-receiver
	receiver_main.c
  receiver_worker.c
)

target_link_libraries(
	mpt-receiver
	mpt-common
	mpt-api-common
	mpt-orchestration-maestro
	mpt-worker
)

install(TARGETS
	mpt-receiver
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


add_executable(
	mpt-receiver-daemon
	receiverd_main.c
	receiverd_worker.c
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
