add_library(
	mpt-worker SHARED
		worker_utils.c
        worker_wait.c
        worker_info.c
		worker_manager.c
		naive_receiver.c
		naive_sender.c
)

target_link_libraries(
        mpt-worker
        mpt-api-common
        mpt-common
)

install(TARGETS mpt-worker
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)




