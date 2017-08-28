if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	message(STATUS "Adding Linux-specific IPC files")
	set(OS_SPECIFIC_IPC_FILES
			ipc/worker_pqueue.c
	)
endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

add_library(
	mpt-worker SHARED
		worker_utils.c
        worker_info.c
		worker_list.c
		worker_data_dump.c
        ipc/worker_vqueue.c
		${OS_SPECIFIC_IPC_FILES}
		ipc/worker_queue.c
		worker_manager.c
		naive_receiver.c
		naive_sender.c
		rate_receiver.c
		rate_sender.c
)

target_link_libraries(
        mpt-worker
        mpt-api-common
        mpt-common
)

install(TARGETS mpt-worker
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)




