add_definitions(-D_GNU_SOURCE -D_POSIX_C_SOURCE)

if (${CMAKE_SYSTEM_NAME} EQUAL "Linux") 
 add_definitions(-DLINUX_BUILD) 
endif (${CMAKE_SYSTEM_NAME} EQUAL "Linux")

set(PROTON_DIR /usr CACHE String "QPID Proton base directory")

include_directories(
	${PROTON_DIR}/include
        .
)

add_library(
	mpt-proton SHARED
        proton-wrapper.c
)





