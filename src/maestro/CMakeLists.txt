add_executable(
    mpt-maestro
    maestro_main.c
    maestro_loop.c
    maestro_cmd_ctxt.c
    maestro_command.c
    maestro_forward_queue.c
    maestro_forward_daemon.c
)

# Readline
find_path(READLINE_INCLUDE_DIR readline/readline.h)
find_library(READLINE_LIB NAMES readline)

message(STATUS "Readline library found at ${READLINE_LIB}")

target_link_libraries(
	mpt-maestro
	mpt-common
        mpt-api-common
        ${READLINE_LIB}
        pthread
)

install(TARGETS
        mpt-maestro
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
