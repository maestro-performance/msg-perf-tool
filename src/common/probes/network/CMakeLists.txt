add_library(
	mpt-probe-net SHARED
        net_probe.c
)

install(TARGETS mpt-probe-net
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
