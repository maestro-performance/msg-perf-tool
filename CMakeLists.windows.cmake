set(CPP_LIBRARY_DIR "lib")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/target/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/target/lib)
set (CTEST_BINARY_DIRECTORY ${CMAKE_BINARY_DIR}/target/lib)

add_subdirectory(deps/uriparser)
include_directories(${CMAKE_BINARY_DIR}/uriparser/include)

set(URIPARSER_LIB "uriparser")
