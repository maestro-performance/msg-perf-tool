add_custom_target(loader-scripts ALL
   COMMAND ${CMAKE_COMMAND} -P loader-tools.cmake
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

install(PROGRAMS mpt-loader.py
	DESTINATION ${RUNTIME_DIR}
)

install(FILES config/mpt-loader.conf config/sample-test-case.conf
	DESTINATION ${MPT_SHARED_DIR}
)

