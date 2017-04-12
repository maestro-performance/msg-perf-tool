if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        set(PROBES_FILES probes/scheduler.c)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

if (UNIX)
        set(MAESTRO_FILES
                maestro/maestro_note.c
                maestro/maestro_instrument.c
                maestro/maestro_sheet.c
                maestro/maestro_player.c)
endif (UNIX)

add_library(
	mpt-common SHARED
        contrib/options.c
        config.c
        vmsl.c
        msgctxt.c
        statistics.c
        process_utils.c
        bmic_utils.c
        msg_content_data.c
        ${PROBES_FILES}
        ${MAESTRO_FILES}
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        add_subdirectory(probes/network objs/common/probes/network)
        add_subdirectory(probes/bmic objs/common/probes/bmic)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

install(TARGETS mpt-common
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)




