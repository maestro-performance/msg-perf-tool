add_library(
	mpt-api-common SHARED
        vmsl_assign.c
)

if (${MQTT_SUPPORT})
    set(MQTT_LIBRARIES mpt-paho)
endif(${MQTT_SUPPORT})

target_link_libraries(
    mpt-api-common
    mpt-common
    ${MQTT_LIBRARIES}
)

install(TARGETS mpt-api-common
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
