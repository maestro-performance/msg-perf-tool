add_custom_target(loader-scripts ALL
	COMMAND ${CMAKE_COMMAND} -P loader-tools.cmake
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

install(PROGRAMS mpt-loader.py
	DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES config/mpt-loader.conf config/sample-test-case.conf
	DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/mpt
)

