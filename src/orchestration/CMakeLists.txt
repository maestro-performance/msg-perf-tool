if (UNIX)
        set(MAESTRO_FILES
                maestro/maestro_note.c
				maestro/maestro_debug.c
                maestro/maestro_instrument.c
                maestro/maestro_sheet.c
                maestro/maestro_player.c
				maestro/maestro_notify.c
                maestro/maestro_serialize.c
                maestro/maestro_easy.c
				maestro/maestro_deserialize.c)
endif (UNIX)

add_library(
	mpt-orchestration-maestro SHARED
        ${MAESTRO_FILES}
)

target_link_libraries(
        mpt-orchestration-maestro
        mpt-common
        mpt-api-common
        pthread
)

install(TARGETS mpt-orchestration-maestro
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)




