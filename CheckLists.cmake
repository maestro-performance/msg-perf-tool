macro (FailIfNotSet VARIABLE name)
	if (NOT ${VARIABLE})
		set(MSG "** A required file, " ${name} ", was not found on your system **")
		message(FATAL_ERROR ${MSG})
	endif (NOT ${VARIABLE})
endmacro (FailIfNotSet)

include (CheckIncludeFiles)
if (UNIX)
	CHECK_INCLUDE_FILES(stdlib.h HAVE_STDLIB_H)
	FailIfNotSet(HAVE_STDLIB_H stdlib.h)

	CHECK_INCLUDE_FILES(stdio.h HAVE_STDIO_H)
	FailIfNotSet(HAVE_STDIO_H stdio.h)

	CHECK_INCLUDE_FILES(stdarg.h HAVE_STDARG_H)
	FailIfNotSet(HAVE_STDARG_H stdarg.h)

	CHECK_INCLUDE_FILES(errno.h HAVE_ERRNO_H)
	FailIfNotSet(HAVE_ERRNO_H errno.h)

	CHECK_INCLUDE_FILES(string.h HAVE_STRING_H)
	FailIfNotSet(HAVE_STRING_H string.h)

	CHECK_INCLUDE_FILES(strings.h HAVE_STRINGS_H)
	FailIfNotSet(HAVE_STRINGS_H strings.h)

	CHECK_INCLUDE_FILES(stdbool.h HAVE_STDBOOL_H)
	FailIfNotSet(HAVE_STDBOOL_H stdbool.h)

	CHECK_INCLUDE_FILES(sys/time.h HAVE_SYS_TIME_H)
	FailIfNotSet(HAVE_SYS_TIME_H sys/time.h)

	CHECK_INCLUDE_FILES(sys/types.h HAVE_SYS_TYPES_H)
	FailIfNotSet(HAVE_SYS_TYPES_H sys/types.h)

	CHECK_INCLUDE_FILES(sys/stat.h HAVE_SYS_STAT_H)
	FailIfNotSet(HAVE_SYS_STAT_H sys/stat.h)

	CHECK_INCLUDE_FILES(inttypes.h HAVE_INT_TYPES_H)
	FailIfNotSet(HAVE_INT_TYPES_H inttypes.h)

	CHECK_INCLUDE_FILES(pthread.h HAVE_PTHREAD_H)
	FailIfNotSet(HAVE_PTHREAD_H pthread.h)

	CHECK_INCLUDE_FILES(fcntl.h HAVE_FCNTL_H)
	FailIfNotSet(HAVE_FCNTL_H fcntl.h)

	CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD_H)
	FailIfNotSet(HAVE_UNISTD_H unistd.h)

	CHECK_INCLUDE_FILES(ctype.h HAVE_CTYPE_H)
	FailIfNotSet(HAVE_CTYPE_H ctype.h)

	CHECK_INCLUDE_FILES(semaphore.h HAVE_SEMAPHORE_H)
	FailIfNotSet(HAVE_SEMAPHORE_H semaphore.h)

	CHECK_INCLUDE_FILES(sys/mman.h HAVE_SYS_MMAN_H)
	FailIfNotSet(HAVE_SYS_MMAN_H sys/mman.h)

	CHECK_INCLUDE_FILES(mqueue.h HAVE_MQUEUE_H)
	FailIfNotSet(HAVE_MQUEUE_H mqueue.h)
endif (UNIX)

macro (DefineIfSet VARIABLE)
	if (${VARIABLE})
		set(MSG "Adding definition " ${VARIABLE} "")
		message(STATUS ${MSG})

		add_definitions(-D${VARIABLE})
	endif (${VARIABLE})
endmacro(DefineIfSet)

include(CheckFunctionExists)
check_function_exists(strlcpy HAVE_STRLCPY)
DefineIfSet(HAVE_STRLCPY)

check_function_exists(strndup HAVE_STRNDUP)
DefineIfSet(HAVE_STRNDUP)