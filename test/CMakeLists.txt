project(mpt-tests)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/target/tests/${CMAKE_INSTALL_BINDIR})

link_libraries(${MATH_LIB} 
    ${GRU_LIB} 
    ${BMIC_BASE_LIB} 
    ${BMIC_MANAGEMENT_LIB} 
    ${BMIC_PRODUCT_LIB} 
    ${URIPARSER_LIB}
    ${UUID_LIB}
    ${ZLIB_LIB}
    rt
    pthread
    mpt-common
)


add_executable(
	shr_data_test
	common/ipc/shr_data_test.c
)

add_test(
	NAME shr-data-test
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/target/tests/bin
	COMMAND shr_data_test
)