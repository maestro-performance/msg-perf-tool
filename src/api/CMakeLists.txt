if (${CMAKE_SYSTEM_NAME} EQUAL "Linux")
 add_definitions(-DLINUX_BUILD)
endif (${CMAKE_SYSTEM_NAME} EQUAL "Linux")

add_library(
	mpt-api-common SHARED
        vmsl_assign.c
)

if (${STOMP_SUPPORT})
  set(STOMP_LIBRARIES mpt-stomp)
endif(${STOMP_SUPPORT})

if (${AMQP_SUPPORT})
  set(AMQP_LIBRARIES mpt-proton)
endif(${AMQP_SUPPORT})

target_link_libraries(
    mpt-api-common
    mpt-common
    ${AMQP_LIBRARIES}
    ${STOMP_LIBRARIES}
)

install(TARGETS mpt-api-common
	LIBRARY DESTINATION ${CPP_LIBRARY_DIR}
)




