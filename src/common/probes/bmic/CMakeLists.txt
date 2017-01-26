add_library(
	mpt-probe-bmic SHARED
        bmic_probe.c
)


install(TARGETS mpt-probe-bmic
	LIBRARY DESTINATION ${CPP_LIBRARY_DIR}
)
