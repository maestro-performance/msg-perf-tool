if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        set(PROBES_FILES probes/scheduler.c)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

add_library(
	mpt-common SHARED
        contrib/options.c
        config.c
        vmsl.c
        msgctxt.c
        statistics.c
        process_utils.c
	msgdata.c
        bmic_utils.c
        ${PROBES_FILES}
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        add_subdirectory(probes/network objs/common/probes/network)
        add_subdirectory(probes/bmic objs/common/probes/bmic)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

install(TARGETS mpt-common
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)




