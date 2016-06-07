add_custom_target(scripts ALL
   COMMAND ${CMAKE_COMMAND} -P plot-tools.cmake
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

install(PROGRAMS
        mpt-plot.sh
	RUNTIME DESTINATION ${RUNTIME_DIR}
)

