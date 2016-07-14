add_custom_target(scripts ALL
   COMMAND ${CMAKE_COMMAND} -P plot-tools.cmake
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

install(PROGRAMS mpt-parse.sh mpt-runner.sh
	DESTINATION ${RUNTIME_DIR}
)

install(FILES latency.ps throughput.ps report.html
	DESTINATION ${MPT_SHARED_DIR}
)

