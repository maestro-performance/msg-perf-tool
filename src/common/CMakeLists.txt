if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        set(PROBES_FILES probes/scheduler.c)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

add_library(
	mpt-common SHARED
        contrib/options.c
        statistics/calculator.c
        statistics/csv_writer.c
        statistics/out_writer.c
        statistics/nop_writer.c
        statistics/naming_utils.c
        ipc/shared_data_buffer.c
        strategies/payload/pl_fixed_strategy.c
        strategies/payload/pl_variable_strategy.c
        bmic/bmic_utils.c
        bmic/bmic_writer.c
        vmsl.c
        vmslh.c
        msg_conn_info.c
        msgctxt.c
        process_utils.c
        msg_content_data.c
        ${PROBES_FILES}
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        add_subdirectory(probes/network objs/common/probes/network)
        add_subdirectory(probes/bmic objs/common/probes/bmic)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

install(TARGETS mpt-common
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)




