project(mpt)
cmake_minimum_required(VERSION 2.8)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/target/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/target/lib)

set(RUNTIME_DIR "bin")
set(CPP_LIBRARY_DIR "lib")
set(CPP_INCLUDE_DIR "include")


if (${CMAKE_SYSTEM_NAME} EQUAL "Linux") 
 add_definitions(-DLINUX_BUILD) 
endif (${CMAKE_SYSTEM_NAME} EQUAL "Linux") 
 

include_directories(src/common)

add_subdirectory(src/api/qpid-proton objs/api/qpid-proton)
add_subdirectory(src/common objs/common)
add_subdirectory(src/receiver objs/receiver)
add_subdirectory(src/sender objs/sender)