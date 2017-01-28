add_library(
	mpt-probe-bmic SHARED
        bmic_probe.c
)

link_libraries(${BMIC_BASE_LIB} ${BMIC_MANAGEMENT_LIB} ${BMIC_PRODUCT_LIB})

install(TARGETS mpt-probe-bmic
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
