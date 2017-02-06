add_library(
	mpt-probe-bmic MODULE
        bmic_probe.c
)

link_libraries(${GRU_LIB} ${BMIC_BASE_LIB} ${BMIC_MANAGEMENT_LIB} ${BMIC_PRODUCT_LIB})

install(TARGETS mpt-probe-bmic
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
