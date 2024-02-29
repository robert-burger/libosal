cmake_minimum_required(VERSION 3.5)
file(READ "project.properties" PROJECT_PROPERTIES)
string(REGEX MATCH "VERSION = ([0-9]*.[0-9]*.[0-9]*)" _ ${PROJECT_PROPERTIES})
set(PROJECT_VERSION ${CMAKE_MATCH_1})
project(libosal VERSION ${PROJECT_VERSION})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CMakePackageConfigHelpers)

option(BUILD_FOR_PLATFORM "Set to WIN32, MINGW32, PIKEOS, POSIX, or VXWORKS" "")
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

if(${BUILD_FOR_PLATFORM} STREQUAL "POSIX")
    set(LIBOSAL_BUILD_POSIX 1)
    set(EXCLUDE_PIKEOS "pikeos")
    set(EXCLUDE_POSIX "vxworks")
    set(EXCLUDE_WIN32 "win32")
    set(SRC_OSAL_POSIX
        src/posix/binary_semaphore.c
        src/posix/condvar.c
        src/posix/io.c
        src/posix/mq.c
        src/posix/mutex.c
        src/posix/semaphore.c
        src/posix/shm.c
        src/posix/spinlock.c
        src/posix/task.c
        src/posix/timer.c
    )
elseif(${BUILD_FOR_PLATFORM} STREQUAL "MINGW32")
    set(LIBOSAL_BUILD_MINGW32 1)
    set(LIBOSAL_BUILD_POSIX 1)
    set(EXCLUDE_PIKEOS "pikeos")
    set(EXCLUDE_VXWORKS "vxworks")
    set(EXCLUDE_WIN32 "win32")
    set(SRC_OSAL_POSIX
        src/posix/binary_semaphore.c
        src/posix/condvar.c
        src/posix/io.c
        src/posix/mq.c
        src/posix/mutex.c
        src/posix/semaphore.c
        src/posix/shm.c
        src/posix/spinlock.c
        src/posix/task.c
        src/posix/timer.c
    )
elseif(${BUILD_FOR_PLATFORM} STREQUAL "WIN32")
    set(LIBOSAL_BUILD_WIN32 1)
    set(EXCLUDE_PIKEOS "pikeos")
    set(EXCLUDE_POSIX "posix")
    set(EXCLUDE_VXWORKS "vxworks")
    set(SRC_OSAL_WIN32
        src/win32/binary_semaphore.c
        src/win32/mutex.c
        src/win32/semaphore.c
    )
elseif(${BUILD_FOR_PLATFORM} STREQUAL "PIKEOS")
    set(LIBOSAL_BUILD_PIKEOS 1)
    set(EXCLUDE_POSIX "posix")
    set(EXCLUDE_VXWORKS "vxworks")
    set(SRC_OSAL_POSIX
        src/pikeos/binary_semaphore.c
        src/pikeos/condvar.c
        src/pikeos/io.c
        src/pikeos/mutex.c
        src/pikeos/osal.c
        src/pikeos/semaphore.c
        src/pikeos/shm.c
        src/pikeos/task.c
        src/pikeos/timer.c
    )
elseif(${BUILD_FOR_PLATFORM} STREQUAL "VXWORKS")
    set(LIBOSAL_BUILD_VXWORKS 1)
    set(EXCLUDE_PIKEOS "pikeos")
    set(EXCLUDE_POSIX "posix")
    set(EXCLUDE_WIN32 "win32")
    set(SRC_OSAL_VXWORKS
        src/vxworks/binary_semaphore.c
        src/vxworks/condvar.c
        src/vxworks/mutex.c
        src/vxworks/semaphore.c
        src/vxworks/task.c
        src/vxworks/timer.c
    )
else()
    message(FATAL_ERROR "Set the BUILD_FOR_PLATFORM option!")
endif()

set(LIBOSAL_STDC_HEADERS 1)
set(LIBOSAL_PACKAGE_VERSION "${CMAKE_PROJECT_VERSION}")
set(LIBOSAL_VERSION "${CMAKE_PROJECT_VERSION}")
set(LIBOSAL_PACKAGE_URL "${CMAKE_PROJECT_HOMEPAGE_URL}")
set(LIBOSAL_PACKAGE "${CMAKE_PROJECT_NAME}")
set(LIBOSAL_PACKAGE_NAME "${CMAKE_PROJECT_NAME}")
set(LIBOSAL_PACKAGE_TARNAME "${CMAKE_PROJECT_NAME}")
set(LIBOSAL_PACKAGE_STRING "${CMAKE_PROJECT_NAME} ${CMAKE_PROJECT_VERSION}")


check_include_files("dlfcn.h" LIBOSAL_HAVE_DLFCN_H)
check_symbol_exists("ENOTRECOVERABLE" "errno.h" LIBOSAL_HAVE_ENOTRECOVERABLE)
check_include_files("inttypes.h" LIBOSAL_HAVE_INTTYPES_H)
check_include_files("math.h" LIBOSAL_HAVE_MATH_H)
check_include_files("mqueue.h" LIBOSAL_HAVE_MQUEUE_H)
check_include_files("p4ext_threads.h" LIBOSAL_HAVE_P4EXT_THREADS_H)
check_symbol_exists("p4_mutext_init_ext"  "p4ext_threads.h" LIBOSAL_HAVE_P4_MUTEX_INIT_EXT)
check_symbol_exists("pthread_mutexattr_setrobust" "pthread.h" LIBOSAL_HAVE_PTHREAD_MUTEXATTR_SETROBUST)
list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
check_symbol_exists("pthread_setaffinity_np" "pthread.h" LIBOSAL_HAVE_PTHREAD_SETAFFINITY_NP)
check_symbol_exists("SIGCONT" "signal.h" LIBOSAL_HAVE_SIGCONT)
check_symbol_exists("SIGSTOP" "signal.h" LIBOSAL_HAVE_SIGSTOP)

check_include_files("stdint.h" LIBOSAL_HAVE_STDINT_H)
check_include_files("stdio.h" LIBOSAL_HAVE_STDIO_H)
check_include_files("stdlib.h" LIBOSAL_HAVE_STDLIB_H)
check_include_files("strings.h" LIBOSAL_HAVE_STRINGS_H)
check_include_files("string.h" LIBOSAL_HAVE_STRING_H)
check_include_files("sys/mman.h" LIBOSAL_HAVE_SYS_MMAN_H)
check_include_files("sys/prctl.h" LIBOSAL_HAVE_SYS_PRCTL_H)
check_include_files("sys/stat.h" LIBOSAL_HAVE_SYS_STAT_H)
check_include_files("sys/types.h" LIBOSAL_HAVE_SYS_TYPES_H)
check_include_files("unistd.h" LIBOSAL_HAVE_UNISTD_H)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/template_config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/libosal/config.h)

set(SRC_OSAL 
    src/io.c
    src/osal.c
    src/timer.c
    src/trace.c

    ${SRC_OSAL_PIKEOS}
    ${SRC_OSAL_POSIX}
    ${SRC_OSAL_VXWORKS}
    ${SRC_OSAL_WIN32}
    )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -z noexecstack")
add_library(osal ${SRC_OSAL})

target_include_directories(osal 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  
        $<INSTALL_INTERFACE:include>)
set_property(TARGET osal PROPERTY POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS})


write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/libosalConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
  )

set(INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
set(OSAL_LIB_NAME osal)

configure_package_config_file(
  ${PROJECT_SOURCE_DIR}/cmake/libosalConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/libosalConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/libosal
  PATH_VARS OSAL_LIB_NAME INSTALL_DIR
  )

install(TARGETS osal EXPORT libosalTargets)
install(
    EXPORT libosalTargets 
    FILE libosalTargets.cmake
    DESTINATION lib/cmake/libosal
)
install(DIRECTORY include/libosal/ DESTINATION include/libosal 
	FILES_MATCHING PATTERN "*.h"
	PATTERN "${EXCLUDE_PIKEOS}" EXCLUDE
    PATTERN "${EXCLUDE_POSIX}" EXCLUDE
    PATTERN "${EXCLUDE_VXWORKS}" EXCLUDE
    PATTERN "${EXCLUDE_WIN32}" EXCLUDE
	)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libosalConfig.cmake" 
                "${CMAKE_CURRENT_BINARY_DIR}/libosalConfigVersion.cmake"
    DESTINATION lib/cmake/libosal)
