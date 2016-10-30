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

if (${STOMP_SUPPORT})
  set(STOMP_LIBRARIES mpt-stomp)
endif(${STOMP_SUPPORT})

if (${AMQP_SUPPORT})
  set(AMQP_LIBRARIES mpt-proton)
endif(${AMQP_SUPPORT})

target_link_libraries(
	mpt-sender
	mpt-common
  ${AMQP_LIBRARIES}
  ${STOMP_LIBRARIES}
)

install(TARGETS
            mpt-sender
	RUNTIME DESTINATION ${RUNTIME_DIR}
)
