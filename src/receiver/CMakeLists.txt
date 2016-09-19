add_definitions(-D_GNU_SOURCE -D_POSIX_C_SOURCE)

if (${CMAKE_SYSTEM_NAME} EQUAL "Linux")
 add_definitions(-DLINUX_BUILD)
endif (${CMAKE_SYSTEM_NAME} EQUAL "Linux")

set(RUNTIME_DIR "bin")

add_executable(
	mpt-receiver
	receiver_main.c
  message_receiver.c
)

if (${STOMP_SUPPORT})
  set(STOMP_LIBRARIES mpt-stomp)
endif(${STOMP_SUPPORT})

if (${AMQP_SUPPORT})
  set(AMQP_LIBRARIES mpt-proton)
endif(${AMQP_SUPPORT})

target_link_libraries(
	mpt-receiver
	mpt-common
  ${AMQP_LIBRARIES}
  ${STOMP_LIBRARIES}
)

install(TARGETS
            mpt-receiver
	RUNTIME DESTINATION ${RUNTIME_DIR}
)
