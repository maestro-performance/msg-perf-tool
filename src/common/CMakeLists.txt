if (${CMAKE_SYSTEM_NAME} EQUAL "Linux")
 add_definitions(-DLINUX_BUILD)
endif (${CMAKE_SYSTEM_NAME} EQUAL "Linux")

add_library(
	mpt-common SHARED
        contrib/options.c
        config.c
        vmsl.c
        msgctxt.c
        statistics.c
        process_utils.c
	msgdata.c
        probes/scheduler.c
)

add_subdirectory(probes/network objs/common/probes/network)

install(TARGETS mpt-common
	LIBRARY DESTINATION ${CPP_LIBRARY_DIR}
)




