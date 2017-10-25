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
    pthread
    mpt-common
    mpt-orchestration-maestro
)


add_executable(
	maestro_serialize_test
	common/maestro_serialize_test.c
)

add_test(
	NAME maestro_serialize_test_ok
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/target/tests/bin
	COMMAND maestro_serialize_test "ok"
)

add_test(
	NAME maestro_serialize_test_set_opt
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/target/tests/bin
	COMMAND maestro_serialize_test "set-opt"
)

add_test(
	NAME maestro_serialize_test_ping_request
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/target/tests/bin
	COMMAND maestro_serialize_test "ping-request"
)

add_test(
	NAME maestro_serialize_test_ping_response
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/target/tests/bin
	COMMAND maestro_serialize_test "ping-response"
)

add_test(
	NAME maestro_serialize_test_stats_response
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/target/tests/bin
	COMMAND maestro_serialize_test "stats-response"
)

add_executable(
		latencies_store_test
		common/latencies_store_test.c
)

add_test(
		NAME latencies_store_test_simple
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/target/tests/bin
		COMMAND latencies_store_test "simple"
)

add_test(
		NAME latencies_store_test_long
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/target/tests/bin
		COMMAND latencies_store_test "long-test"
)