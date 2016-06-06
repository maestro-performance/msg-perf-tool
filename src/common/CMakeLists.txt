add_definitions(-D_GNU_SOURCE -D_POSIX_C_SOURCE)

if (${CMAKE_SYSTEM_NAME} EQUAL "Linux") 
 add_definitions(-DLINUX_BUILD) 
endif (${CMAKE_SYSTEM_NAME} EQUAL "Linux") 

add_library(
	mpt-common SHARED
        contrib/options.c
        contrib/logger.c
        contrib/log.c
        vmsl.c
        msgctxt.c
        statistics.c
        process_utils.c
)





