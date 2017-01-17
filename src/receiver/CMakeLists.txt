set(RUNTIME_DIR "bin")

add_executable(
	mpt-receiver
	receiver_main.c
  message_receiver.c
)

target_link_libraries(
	mpt-receiver
	mpt-common
        mpt-api-common
)

install(TARGETS
            mpt-receiver
	RUNTIME DESTINATION ${RUNTIME_DIR}
)
