add_library(
	mpt-worker SHARED
        abstract_worker.c
        worker_utils.c
        worker_wait.c
)

target_link_libraries(
        mpt-worker
        mpt-api-common
        mpt-common
)

install(TARGETS mpt-worker
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)




