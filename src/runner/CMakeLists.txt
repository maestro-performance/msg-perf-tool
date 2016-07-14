add_custom_target(runner-scripts ALL
   COMMAND ${CMAKE_COMMAND} -P runner-tools.cmake
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

install(PROGRAMS mpt-runner.sh
	DESTINATION ${RUNTIME_DIR}
)

