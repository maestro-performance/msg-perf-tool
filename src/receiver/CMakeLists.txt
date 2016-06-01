add_definitions(-D_GNU_SOURCE -D_POSIX_C_SOURCE)

if (${CMAKE_SYSTEM_NAME} EQUAL "Linux") 
 add_definitions(-DLINUX_BUILD) 
endif (${CMAKE_SYSTEM_NAME} EQUAL "Linux")

set(RUNTIME_DIR "bin")

add_executable(
	mpt-receiver
	receiver.c
        message_receiver.c
)

target_link_libraries(
	mpt-receiver
	mpt-common
        mpt-proton
)

install(TARGETS
            mpt-receiver
	RUNTIME DESTINATION ${RUNTIME_DIR}
)

