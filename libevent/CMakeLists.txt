#
# Libevent CMake project
#
# Based on initial work by:
#    Alexey Ozeritsky
#
# Additional changes:
#   Brodie Thiesfield
#   Joakim Soderberg
#   Trond Norbye
#   Sergei Nikulov
#
#   Build example:
#
#       cd libevent
#       md build
#       cd build
#       cmake -G "Visual Studio 10" ..
#       start libevent.sln
#
if (APPLE)
    cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
else()
    cmake_minimum_required(VERSION 2.8.3 FATAL_ERROR)
endif()
cmake_policy(SET CMP0054 NEW)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release
        CACHE STRING "Set build type to Debug o Release (default Release)" FORCE)
endif()
string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)

# get rid of the extra default configurations
# what? why would you get id of other useful build types? - Ellzey
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Limited configurations" FORCE)

project(libevent C)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")
string(REGEX MATCH "SunOS" SOLARIS "${CMAKE_SYSTEM_NAME}")


include(CheckTypeSize)
include(CheckFunctionExistsEx)
include(CheckFileOffsetBits)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckVariableExists)
include(CheckSymbolExists)
include(CheckStructHasMember)
include(CheckCSourceCompiles)
include(CheckPrototypeDefinition)
include(CheckFunctionKeywords)
include(CheckConstExists)
include(AddCompilerFlags)

set(EVENT_ABI_MAJOR ${EVENT_VERSION_MAJOR})
set(EVENT_ABI_MINOR ${EVENT_VERSION_MINOR})
set(EVENT_ABI_PATCH ${EVENT_VERSION_PATCH})

set(EVENT_ABI_LIBVERSION
    "${EVENT_ABI_MAJOR}.${EVENT_ABI_MINOR}.${EVENT_ABI_PATCH}")

set(EVENT_PACKAGE_VERSION
    "${EVENT_VERSION_MAJOR}.${EVENT_VERSION_MINOR}.${EVENT_VERSION_PATCH}")

set(EVENT_NUMERIC_VERSION 0x02020001)

# only a subset of names can be used, defaults to "beta"
set(EVENT_STAGE_NAME ${EVENT_VERSION_STAGE})

# a list that defines what can set for EVENT_STAGE_VERSION
set(EVENT__ALLOWED_STAGE_NAMES
	rc
	beta
	alpha
	alpha-dev
	release
	stable
)
list(
	FIND EVENT__ALLOWED_STAGE_NAMES
	"${EVENT_STAGE_NAME}"
	EVENT__STAGE_RET
)

set(EVENT_VERSION
	"${EVENT_VERSION_MAJOR}.${EVENT_VERSION_MINOR}.${EVENT_VERSION_PATCH}-${EVENT_STAGE_NAME}")

option(EVENT__DISABLE_DEBUG_MODE
    "Define if libevent should build without support for a debug mode" OFF)

option(EVENT__ENABLE_VERBOSE_DEBUG
    "Enables verbose debugging" OFF)

option(EVENT__DISABLE_MM_REPLACEMENT
    "Define if libevent should not allow replacing the mm functions" OFF)

option(EVENT__DISABLE_THREAD_SUPPORT
    "Define if libevent should not be compiled with thread support" OFF)

option(EVENT__DISABLE_BENCHMARK
    "Defines if libevent should build without the benchmark exectuables" ON)

option(EVENT__DISABLE_TESTS
    "If tests should be compiled or not" ON)

option(EVENT__DISABLE_REGRESS
    "Disable the regress tests" ON)

option(EVENT__DISABLE_SAMPLES
    "Disable sample files" ON)

option(EVENT__DISABLE_CLOCK_GETTIME
    "Do not use clock_gettime even if it is available" OFF)

option(EVENT__FORCE_KQUEUE_CHECK
    "When crosscompiling forces running a test program that verifies that Kqueue works with pipes. Note that this requires you to manually run the test program on the the cross compilation target to verify that it works. See cmake documentation for try_run for more details" OFF)

# TODO: Add --disable-largefile     omit support for large files
option(EVENT__COVERAGE
"Enable running gcov to get a test coverage report (only works with GCC/CLang). Make sure to enable -DCMAKE_BUILD_TYPE=Debug as well." OFF)

# Put the libaries and binaries that get built into directories at the
# top of the build tree rather than in hard-to-find leaf directories.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

if (EVENT__ENABLE_VERBOSE_DEBUG)
    add_definitions(-DUSE_DEBUG=1)
endif()

# Setup compiler flags for coverage.
if (EVENT__COVERAGE)
    if (NOT "${CMAKE_BUILD_TYPE_LOWER}" STREQUAL "debug")
        message(FATAL_ERROR "Coverage requires -DCMAKE_BUILD_TYPE=Debug")
    endif()

    message(STATUS "Setting coverage compiler flags")

    set(CMAKE_REQUIRED_LIBRARIES "--coverage")
    add_compiler_flags(-g -O0 --coverage)
    set(CMAKE_REQUIRED_LIBRARIES "")
endif()

# GCC specific options.
if (CMAKE_COMPILER_IS_GNUCC)
    option(EVENT__DISABLE_GCC_WARNINGS "Disable verbose warnings with GCC" OFF)
    option(EVENT__ENABLE_GCC_HARDENING "Enable compiler security checks" OFF)
    option(EVENT__ENABLE_GCC_FUNCTION_SECTIONS "Enable gcc function sections" OFF)
    option(EVENT__ENABLE_GCC_WARNINGS "Make all GCC warnings into errors" OFF)

    list(APPEND __FLAGS -Wall -Wswitch)

    if (EVENT__DISABLE_GCC_WARNINGS)
        list(APPEND __FLAGS -w)
    endif()

    if (EVENT__ENABLE_GCC_HARDENING)
        list(APPEND __FLAGS
             -fstack-protector-all
             -fwrapv
             -fPIE
             -Wstack-protector
             "--param ssp-buffer-size=1")

        add_definitions(-D_FORTIFY_SOURCE=2)
    endif()

    if (EVENT__ENABLE_GCC_FUNCTION_SECTIONS)
        list(APPEND __FLAGS -ffunction-sections)
        # TODO: Add --gc-sections support. We need some checks for NetBSD to ensure this works.
    endif()

    if (EVENT__ENABLE_GCC_WARNINGS)
        list(APPEND __FLAGS -Werror)
    endif()

    # We need to test for at least gcc 2.95 here, because older versions don't
    # have -fno-strict-aliasing
    list(APPEND __FLAGS -fno-strict-aliasing)

    add_compiler_flags(${__FLAGS})
endif()

if (APPLE)
    # Get rid of deprecated warnings for OpenSSL on OSX 10.7 and greater.
    add_compiler_flags(
        -Wno-error=deprecated-declarations
        -Qunused-arguments
    )
endif()

# Winsock.
if(WIN32)
    set(CMAKE_EXTRA_INCLUDE_FILES winsock2.h ws2tcpip.h)
    set(CMAKE_REQUIRED_LIBRARIES  ws2_32.lib)
    set(CMAKE_REQUIRED_DEFINITIONS -FIwinsock2.h -FIws2tcpip.h)
endif()
if (SOLARIS)
    set(CMAKE_REQUIRED_LIBRARIES socket nsl)
endif()

# Check if _GNU_SOURCE is available.
if (NOT _GNU_SOURCE)
  CHECK_SYMBOL_EXISTS(__GNU_LIBRARY__ "features.h" _GNU_SOURCE)

  if (NOT _GNU_SOURCE)
    unset(_GNU_SOURCE CACHE)
    CHECK_SYMBOL_EXISTS(_GNU_SOURCE "features.h" _GNU_SOURCE)
  endif()
endif()

if (_GNU_SOURCE)
    add_definitions(-D_GNU_SOURCE)
endif()

CHECK_INCLUDE_FILE(sys/types.h EVENT__HAVE_SYS_TYPES_H)
if(EVENT__HAVE_SYS_TYPES_H)
    list(APPEND CMAKE_EXTRA_INCLUDE_FILES sys/types.h)
endif()

CHECK_INCLUDE_FILE(sys/socket.h EVENT__HAVE_SYS_SOCKET_H)
if(EVENT__HAVE_SYS_SOCKET_H)
    list(APPEND CMAKE_EXTRA_INCLUDE_FILES sys/socket.h)
endif()

CHECK_INCLUDE_FILE(netinet/in.h EVENT__HAVE_NETINET_IN_H)
if(EVENT__HAVE_NETINET_IN_H)
    list(APPEND CMAKE_EXTRA_INCLUDE_FILES netinet/in.h)
endif()

CHECK_INCLUDE_FILE(netinet/in6.h EVENT__HAVE_NETINET_IN6_H)
if(EVENT__HAVE_NETINET_IN6_H)
    list(APPEND CMAKE_EXTRA_INCLUDE_FILES netinet/in6.h)
endif()

CHECK_INCLUDE_FILE(unistd.h EVENT__HAVE_UNISTD_H)
CHECK_INCLUDE_FILE(netdb.h EVENT__HAVE_NETDB_H)
CHECK_INCLUDE_FILE(dlfcn.h EVENT__HAVE_DLFCN_H)
CHECK_INCLUDE_FILE(arpa/inet.h EVENT__HAVE_ARPA_INET_H)
CHECK_INCLUDE_FILE(fcntl.h EVENT__HAVE_FCNTL_H)
if(EVENT__HAVE_FCNTL_H)
    list(APPEND CMAKE_EXTRA_INCLUDE_FILES fcntl.h)
endif()
CHECK_INCLUDE_FILE(inttypes.h EVENT__HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE(memory.h EVENT__HAVE_MEMORY_H)
CHECK_INCLUDE_FILE(poll.h EVENT__HAVE_POLL_H)
CHECK_INCLUDE_FILE(port.h EVENT__HAVE_PORT_H)
if(EVENT__HAVE_PORT_H)
    list(APPEND CMAKE_EXTRA_INCLUDE_FILES port.h)
endif()
CHECK_INCLUDE_FILE(signal.h EVENT__HAVE_SIGNAL_H)
CHECK_INCLUDE_FILE(stdarg.h EVENT__HAVE_STDARG_H)
CHECK_INCLUDE_FILE(stddef.h EVENT__HAVE_STDDEF_H)
CHECK_INCLUDE_FILE(stdint.h EVENT__HAVE_STDINT_H)
CHECK_INCLUDE_FILE(stdlib.h EVENT__HAVE_STDLIB_H)
CHECK_INCLUDE_FILE(strings.h EVENT__HAVE_STRINGS_H)
CHECK_INCLUDE_FILE(string.h EVENT__HAVE_STRING_H)
CHECK_INCLUDE_FILE(sys/devpoll.h EVENT__HAVE_SYS_DEVPOLL_H)
CHECK_INCLUDE_FILE(sys/epoll.h EVENT__HAVE_SYS_EPOLL_H)
CHECK_INCLUDE_FILE(sys/eventfd.h EVENT__HAVE_SYS_EVENTFD_H)
CHECK_INCLUDE_FILE(sys/event.h EVENT__HAVE_SYS_EVENT_H)
CHECK_INCLUDE_FILE(sys/ioctl.h EVENT__HAVE_SYS_IOCTL_H)
CHECK_INCLUDE_FILE(sys/mman.h EVENT__HAVE_SYS_MMAN_H)
CHECK_INCLUDE_FILE(sys/param.h EVENT__HAVE_SYS_PARAM_H)
CHECK_INCLUDE_FILE(sys/queue.h EVENT__HAVE_SYS_QUEUE_H)
CHECK_INCLUDE_FILE(sys/select.h EVENT__HAVE_SYS_SELECT_H)
CHECK_INCLUDE_FILE(sys/sendfile.h EVENT__HAVE_SYS_SENDFILE_H)
CHECK_INCLUDE_FILE(sys/stat.h EVENT__HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE(sys/time.h EVENT__HAVE_SYS_TIME_H)
if(EVENT__HAVE_SYS_TIME_H)
    list(APPEND CMAKE_EXTRA_INCLUDE_FILES sys/time.h)
endif()
CHECK_INCLUDE_FILE(sys/uio.h EVENT__HAVE_SYS_UIO_H)
CHECK_INCLUDE_FILES("sys/types.h;ifaddrs.h" EVENT__HAVE_IFADDRS_H)
CHECK_INCLUDE_FILE(mach/mach_time.h EVENT__HAVE_MACH_MACH_TIME_H)
CHECK_INCLUDE_FILE(netinet/tcp.h EVENT__HAVE_NETINET_TCP_H)
CHECK_INCLUDE_FILE(sys/wait.h EVENT__HAVE_SYS_WAIT_H)
CHECK_INCLUDE_FILE(sys/resource.h EVENT__HAVE_SYS_RESOURCE_H)
CHECK_INCLUDE_FILE(sys/sysctl.h EVENT__HAVE_SYS_SYSCTL_H)
CHECK_INCLUDE_FILE(sys/timerfd.h EVENT__HAVE_SYS_TIMERFD_H)
CHECK_INCLUDE_FILE(errno.h EVENT__HAVE_ERRNO_H)


CHECK_FUNCTION_EXISTS_EX(epoll_create EVENT__HAVE_EPOLL)
CHECK_FUNCTION_EXISTS_EX(epoll_ctl EVENT__HAVE_EPOLL_CTL)
CHECK_FUNCTION_EXISTS_EX(eventfd EVENT__HAVE_EVENTFD)
if(NOT EVENT__DISABLE_CLOCK_GETTIME)
    CHECK_FUNCTION_EXISTS_EX(clock_gettime EVENT__HAVE_CLOCK_GETTIME)
endif()
CHECK_FUNCTION_EXISTS_EX(fcntl EVENT__HAVE_FCNTL)
CHECK_FUNCTION_EXISTS_EX(getaddrinfo EVENT__HAVE_GETADDRINFO)
CHECK_FUNCTION_EXISTS_EX(getnameinfo EVENT__HAVE_GETNAMEINFO)
CHECK_FUNCTION_EXISTS_EX(gettimeofday EVENT__HAVE_GETTIMEOFDAY)
CHECK_FUNCTION_EXISTS_EX(getprotobynumber EVENT__HAVE_GETPROTOBYNUMBER)
CHECK_FUNCTION_EXISTS_EX(getservbyname EVENT__HAVE_GETSERVBYNAME)
CHECK_FUNCTION_EXISTS_EX(inet_ntop EVENT__HAVE_INET_NTOP)
CHECK_FUNCTION_EXISTS_EX(inet_pton EVENT__HAVE_INET_PTON)
CHECK_FUNCTION_EXISTS_EX(kqueue EVENT__HAVE_KQUEUE)
CHECK_FUNCTION_EXISTS_EX(mmap EVENT__HAVE_MMAP)
CHECK_FUNCTION_EXISTS_EX(pipe EVENT__HAVE_PIPE)
CHECK_FUNCTION_EXISTS_EX(pipe2 EVENT__HAVE_PIPE2)
CHECK_FUNCTION_EXISTS_EX(poll EVENT__HAVE_POLL)
CHECK_FUNCTION_EXISTS_EX(port_create EVENT__HAVE_PORT_CREATE)
CHECK_FUNCTION_EXISTS_EX(sendfile EVENT__HAVE_SENDFILE)
CHECK_FUNCTION_EXISTS_EX(sigaction EVENT__HAVE_SIGACTION)
CHECK_FUNCTION_EXISTS_EX(signal EVENT__HAVE_SIGNAL)
CHECK_FUNCTION_EXISTS_EX(splice EVENT__HAVE_SPLICE)
CHECK_FUNCTION_EXISTS_EX(strlcpy EVENT__HAVE_STRLCPY)
CHECK_FUNCTION_EXISTS_EX(strsep EVENT__HAVE_STRSEP)
CHECK_FUNCTION_EXISTS_EX(strtok_r EVENT__HAVE_STRTOK_R)
CHECK_FUNCTION_EXISTS_EX(strtoll EVENT__HAVE_STRTOLL)
CHECK_FUNCTION_EXISTS_EX(vasprintf EVENT__HAVE_VASPRINTF)
CHECK_FUNCTION_EXISTS_EX(sysctl EVENT__HAVE_SYSCTL)
CHECK_FUNCTION_EXISTS_EX(accept4 EVENT__HAVE_ACCEPT4)
CHECK_FUNCTION_EXISTS_EX(arc4random EVENT__HAVE_ARC4RANDOM)
CHECK_FUNCTION_EXISTS_EX(arc4random_buf EVENT__HAVE_ARC4RANDOM_BUF)
CHECK_FUNCTION_EXISTS_EX(arc4random_addrandom EVENT__HAVE_ARC4RANDOM_ADDRANDOM)
CHECK_FUNCTION_EXISTS_EX(epoll_create1 EVENT__HAVE_EPOLL_CREATE1)
CHECK_FUNCTION_EXISTS_EX(getegid EVENT__HAVE_GETEGID)
CHECK_FUNCTION_EXISTS_EX(geteuid EVENT__HAVE_GETEUID)
CHECK_FUNCTION_EXISTS_EX(getifaddrs EVENT__HAVE_GETIFADDRS)
CHECK_FUNCTION_EXISTS_EX(issetugid EVENT__HAVE_ISSETUGID)
CHECK_FUNCTION_EXISTS_EX(mach_absolute_time EVENT__HAVE_MACH_ABSOLUTE_TIME)
CHECK_FUNCTION_EXISTS_EX(nanosleep EVENT__HAVE_NANOSLEEP)
CHECK_FUNCTION_EXISTS_EX(usleep EVENT__HAVE_USLEEP)
CHECK_FUNCTION_EXISTS_EX(timeradd EVENT__HAVE_TIMERADD)
CHECK_FUNCTION_EXISTS_EX(timerclear EVENT__HAVE_TIMERCLEAR)
CHECK_FUNCTION_EXISTS_EX(timercmp EVENT__HAVE_TIMERCMP)
CHECK_FUNCTION_EXISTS_EX(timerfd_create EVENT__HAVE_TIMERFD_CREATE)
CHECK_FUNCTION_EXISTS_EX(timerisset EVENT__HAVE_TIMERISSET)
CHECK_FUNCTION_EXISTS_EX(putenv EVENT__HAVE_PUTENV)
CHECK_FUNCTION_EXISTS_EX(setenv EVENT__HAVE_SETENV)
CHECK_FUNCTION_EXISTS_EX(setrlimit EVENT__HAVE_SETRLIMIT)
CHECK_FUNCTION_EXISTS_EX(umask EVENT__HAVE_UMASK)
CHECK_FUNCTION_EXISTS_EX(unsetenv EVENT__HAVE_UNSETENV)

# Get the gethostbyname_r prototype.
CHECK_FUNCTION_EXISTS_EX(gethostbyname_r EVENT__HAVE_GETHOSTBYNAME_R)

if(EVENT__HAVE_GETHOSTBYNAME_R)
    CHECK_PROTOTYPE_DEFINITION(gethostbyname_r
        "int gethostbyname_r(const char *name, struct hostent *hp, struct hostent_data *hdata)"
        "0"
        "netdb.h"
        EVENT__HAVE_GETHOSTBYNAME_R_3_ARG)

    CHECK_PROTOTYPE_DEFINITION(gethostbyname_r
        "struct hostent *gethostbyname_r(const char *name, struct hostent *hp, char *buf, size_t buflen, int *herr)"
        "NULL"
        "netdb.h"
        EVENT__HAVE_GETHOSTBYNAME_R_5_ARG)

    CHECK_PROTOTYPE_DEFINITION(gethostbyname_r
        "int gethostbyname_r(const char *name, struct hostent *hp, char *buf, size_t buflen, struct hostent **result, int *herr)"
        "0"
        "netdb.h"
        EVENT__HAVE_GETHOSTBYNAME_R_6_ARG)
endif()

if(HAVE_PORT_H AND HAVE_PORT_CREATE)
    set(EVENT__HAVE_EVENT_PORTS 1)
endif()

if(NOT WIN32)
    CHECK_FUNCTION_EXISTS_EX(select EVENT__HAVE_SELECT)
endif()

CHECK_TYPE_SIZE("uint8_t" EVENT__HAVE_UINT8_T)
CHECK_TYPE_SIZE("uint16_t" EVENT__HAVE_UINT16_T)
CHECK_TYPE_SIZE("uint32_t" EVENT__HAVE_UINT32_T)
CHECK_TYPE_SIZE("uint64_t" EVENT__HAVE_UINT64_T)
CHECK_TYPE_SIZE("short" EVENT__SIZEOF_SHORT BUILTIN_TYPES_ONLY)
CHECK_TYPE_SIZE("int" EVENT__SIZEOF_INT BUILTIN_TYPES_ONLY)
CHECK_TYPE_SIZE("unsigned" EVENT__SIZEOF_UNSIGNED BUILTIN_TYPES_ONLY)
CHECK_TYPE_SIZE("unsigned int" EVENT__SIZEOF_UNSIGNED_INT BUILTIN_TYPES_ONLY)
CHECK_TYPE_SIZE("long" EVENT__SIZEOF_LONG BUILTIN_TYPES_ONLY)
CHECK_TYPE_SIZE("long long" EVENT__SIZEOF_LONG_LONG BUILTIN_TYPES_ONLY)

if(WIN32)
    # These aren't available until Windows Vista.
    # But you can still link them. They just won't be found when running the exe.
    set(EVENT__HAVE_INET_NTOP 0)
    set(EVENT__HAVE_INET_PTON 0)
endif()

# Check for different inline keyword versions.
check_function_keywords("inline" "__inline" "__inline__")

if (HAVE_INLINE)
    set (EVENT__inline inline)
elseif (HAVE___INLINE)
    set(EVENT__inline __inline)
elseif(HAVE___INLINE__)
    set(EVENT__inline __inline__)
else()
    set(EVENT__inline)
endif()

CHECK_SYMBOL_EXISTS(TAILQ_FOREACH sys/queue.h EVENT__HAVE_TAILQFOREACH)
CHECK_CONST_EXISTS(CTL_KERN sys/sysctl.h EVENT__HAVE_DECL_CTL_KERN)
CHECK_CONST_EXISTS(KERN_ARND sys/sysctl.h EVENT__HAVE_DECL_KERN_ARND)
CHECK_CONST_EXISTS(KERN_RANDOM sys/sysctl.h EVENT__HAVE_DECL_KERN_RANDOM)
CHECK_CONST_EXISTS(RANDOM_UUID sys/sysctl.h EVENT__HAVE_DECL_RANDOM_UUID)
CHECK_SYMBOL_EXISTS(F_SETFD fcntl.h EVENT__HAVE_SETFD)

CHECK_TYPE_SIZE(fd_mask EVENT__HAVE_FD_MASK)

CHECK_TYPE_SIZE(size_t EVENT__SIZEOF_SIZEE_T)
if(NOT EVENT__SIZEOF_SIZE_T)
  set(EVENT__size_t "unsigned")
  set(EVENT__SIZEOF_SIZE_T ${EVENT__SIZEOF_UNSIGNED})
else()
    set(EVENT__size_t size_t)
endif()

CHECK_TYPE_SIZE("off_t" EVENT__SIZEOF_OFF_T LANGUAGE C)


# XXX we should functionalize these size and type sets. --elley

# Winssck.
if (_MSC_VER)
    list(APPEND CMAKE_EXTRA_INCLUDE_FILES BaseTsd.h)
endif()
CHECK_TYPE_SIZE("ssize_t" EVENT__SIZEOF_SSIZE_T_LOWER LANGUAGE C)
CHECK_TYPE_SIZE("SSIZE_T" EVENT__SIZEOF_SSIZE_T_UPPER LANGUAGE C)

if (EVENT__SIZEOF_SSIZE_T_LOWER)
    set(EVENT__ssize_t "ssize_t")
    set(EVENT__SIZEOF_SSIZE_T ${EVENT__SIZEOF_SSIZE_T_LOWER})
elseif (EVENT__SIZEOF_SSIZE_T_UPPER)
    set(EVENT__ssize_t "SSIZE_T")
    set(EVENT__SIZEOF_SSIZE_T ${EVENT__SIZEOF_SSIZE_T_UPPER})
else()
    set(EVENT__ssize_t "int")
    set(EVENT__SIZEOF_SSIZE_T ${EVENT__SIZEOF_INT})
endif()


CHECK_TYPE_SIZE(socklen_t EVENT__SIZEOF_SOCKLEN_T)
if(NOT EVENT__SIZEOF_SOCKLEN_T)
  set(EVENT__socklen_t "unsigned int")
  set(EVENT__SIZEOF_SOCKLEN_T ${EVENT__SIZEOF_UNSIGNED_INT})
else()
    set(EVENT__socklen_t "socklen_t")
endif()

CHECK_TYPE_SIZE(pid_t EVENT__SIZEOF_PID_T)
if(NOT EVENT__SIZEOF_PID_T)
  set(EVENT__pid_t "int")
  set(EVENT__SIZEOF_PID_T ${EVENT__SIZEOF_INT})
else()
	set(EVENT__pid_t "pid_t")
	set(EVENT__SIZEOF_PID_T EVENT__SIZEOF_PID_T)
endif()

if (NOT EVENT__DISABLE_THREAD_SUPPORT)
    if (NOT WIN32)
        list(APPEND CMAKE_EXTRA_INCLUDE_FILES pthread.h)
    endif()
    CHECK_TYPE_SIZE(pthread_t EVENT__SIZEOF_PTHREAD_T)
endif()

if(EVENT__HAVE_CLOCK_GETTIME)
  set(EVENT__DNS_USE_CPU_CLOCK_FOR_ID 1)
endif()

# we're just getting lazy now.
CHECK_TYPE_SIZE("uintptr_t" EVENT__HAVE_UINTPTR_T)
CHECK_TYPE_SIZE("void *" EVENT__SIZEOF_VOID_P)

# Tests file offset bits.
# TODO: Add AIX test for if -D_LARGE_FILES is needed.

# XXX: Why is this here? we don't even use it. Well, we don't even use it
#      on top of that, why is it set in the config.h?! IT_MAKES_NO_SENSE
#      I'm commenting it out for now.
#      - ellzey

#CHECK_FILE_OFFSET_BITS()

# Verify kqueue works with pipes.
if (EVENT__HAVE_KQUEUE)
    if (CMAKE_CROSSCOMPILING AND NOT EVENT__FORCE_KQUEUE_CHECK)
        message(WARNING "Cannot check if kqueue works with pipes when crosscompiling, use EVENT__FORCE_KQUEUE_CHECK to be sure (this requires manually running a test program on the cross compilation target)")
        set(EVENT__HAVE_WORKING_KQUEUE 1)
    else()
        message(STATUS "Checking if kqueue works with pipes...")
        include(CheckWorkingKqueue)
    endif()
endif()

CHECK_SYMBOL_EXISTS(_MINIX "stdio.h" EVENT___MINIX)
CHECK_SYMBOL_EXISTS(_POSIX_1_SOURCE "stdio.h" EVENT___POSIX_1_SOURCE)
CHECK_SYMBOL_EXISTS(_POSIX_SOURCE "stdio.h" EVENT___POSIX_SOURCE)

if(EVENT__HAVE_NETDB_H)
    list(APPEND CMAKE_EXTRA_INCLUDE_FILES netdb.h)
    CHECK_TYPE_SIZE("struct addrinfo" EVENT__HAVE_STRUCT_ADDRINFO)
elseif(WIN32)
    list(APPEND CMAKE_EXTRA_INCLUDE_FILES ws2tcpip.h)
    CHECK_TYPE_SIZE("struct addrinfo" EVENT__HAVE_STRUCT_ADDRINFO)
endif()

# Check for sockaddr structure sizes.
set(SOCKADDR_HEADERS)
if (WIN32)
    configure_msvc_runtime()
    set(CMAKE_REQUIRED_DEFINITIONS "-DWIN32_LEAN_AND_MEAN")
    if (_MSC_VER LESS 1300)
        set(SOCKADDR_HEADERS winsock.h)
    else()
        set(SOCKADDR_HEADERS winsock2.h ws2tcpip.h)
    endif()
else()
    if (EVENT__HAVE_NETINET_IN_H)
        set(SOCKADDR_HEADERS ${SOCKADDR_HEADERS} netinet/in.h)
    endif()

    if (EVENT__HAVE_NETINET_IN6_H)
        set(SOCKADDR_HEADERS ${SOCKADDR_HEADERS} netinet/in6.h)
    endif()

    if (EVENT__HAVE_SYS_SOCKET_H)
        set(SOCKADDR_HEADERS ${SOCKADDR_HEADERS} sys/socket.h)
    endif()

    if (EVENT__HAVE_NETDB_H)
        set(SOCKADDR_HEADERS ${SOCKADDR_HEADERS} netdb.h)
    endif()
endif()

CHECK_TYPE_SIZE("struct in6_addr" EVENT__HAVE_STRUCT_IN6_ADDR)
if(EVENT__HAVE_STRUCT_IN6_ADDR)
    CHECK_STRUCT_HAS_MEMBER("struct in6_addr"
            s6_addr16 "${SOCKADDR_HEADERS}"
            EVENT__HAVE_STRUCT_IN6_ADDR_S6_ADDR16)

    CHECK_STRUCT_HAS_MEMBER("struct in6_addr"
            s6_addr32 "${SOCKADDR_HEADERS}"
            EVENT__HAVE_STRUCT_IN6_ADDR_S6_ADDR32)
endif()

CHECK_TYPE_SIZE("sa_family_t" EVENT__HAVE_SA_FAMILY_T)
CHECK_TYPE_SIZE("struct sockaddr_in6" EVENT__HAVE_STRUCT_SOCKADDR_IN6)

if(EVENT__HAVE_STRUCT_SOCKADDR_IN6)
    CHECK_STRUCT_HAS_MEMBER("struct sockaddr_in6"
            sin6_len "${SOCKADDR_HEADERS}"
            EVENT__HAVE_STRUCT_SOCKADDR_IN6_SIN6_LEN)

    CHECK_STRUCT_HAS_MEMBER("struct sockaddr_in6"
            sin_len "${SOCKADDR_HEADERS}"
            EVENT__HAVE_STRUCT_SOCKADDR_IN_SIN_LEN)
endif()

CHECK_TYPE_SIZE("struct sockaddr_storage" EVENT__HAVE_STRUCT_SOCKADDR_STORAGE)
if(EVENT__HAVE_STRUCT_SOCKADDR_STORAGE)
    CHECK_STRUCT_HAS_MEMBER("struct sockaddr_storage"
            ss_family "${SOCKADDR_HEADERS}"
            EVENT__HAVE_STRUCT_SOCKADDR_STORAGE_SS_FAMILY)

    CHECK_STRUCT_HAS_MEMBER("struct sockaddr_storage"
            __ss_family "${SOCKADDR_HEADERS}" EVENT__HAVE_STRUCT_SOCKADDR_STORAGE___SS_FAMILY)
endif()

CHECK_TYPE_SIZE("struct linger" EVENT__HAVE_STRUCT_LINGER)

# Group the source files.
set(HDR_PRIVATE
    bufferevent-internal.h
    changelist-internal.h
    defer-internal.h
    epolltable-internal.h
    evbuffer-internal.h
    event-internal.h
    evmap-internal.h
    evrpc-internal.h
    evsignal-internal.h
    evthread-internal.h
    ht-internal.h
    http-internal.h
    iocp-internal.h
    ipv6-internal.h
    log-internal.h
    minheap-internal.h
    mm-internal.h
    ratelim-internal.h
    strlcpy-internal.h
    util-internal.h
    evconfig-private.h
    compat/sys/queue.h)

set(HDR_COMPAT
    include/evdns.h
    include/evrpc.h
    include/event.h
    include/evhttp.h
    include/evutil.h)

set(HDR_PUBLIC
    include/event2/buffer.h
    include/event2/bufferevent.h
    include/event2/bufferevent_compat.h
    include/event2/bufferevent_struct.h
    include/event2/buffer_compat.h
    include/event2/dns.h
    include/event2/dns_compat.h
    include/event2/dns_struct.h
    include/event2/event.h
    include/event2/event_compat.h
    include/event2/event_struct.h
    include/event2/http.h
    include/event2/http_compat.h
    include/event2/http_struct.h
    include/event2/keyvalq_struct.h
    include/event2/listener.h
    include/event2/rpc.h
    include/event2/rpc_compat.h
    include/event2/rpc_struct.h
    include/event2/tag.h
    include/event2/tag_compat.h
    include/event2/thread.h
    include/event2/util.h
    include/event2/visibility.h
    ${PROJECT_BINARY_DIR}/include/event2/event-config.h)

set(SRC_CORE
    buffer.c
    bufferevent.c
    bufferevent_filter.c
    bufferevent_pair.c
    bufferevent_ratelim.c
    bufferevent_sock.c
    event.c
    evmap.c
    evthread.c
    evutil.c
    evutil_rand.c
    evutil_time.c
    listener.c
    log.c
    signal.c
    strlcpy.c)

if(EVENT__HAVE_SELECT)
    list(APPEND SRC_CORE select.c)
endif()

if(EVENT__HAVE_POLL)
    list(APPEND SRC_CORE poll.c)
endif()

if(EVENT__HAVE_KQUEUE)
    list(APPEND SRC_CORE kqueue.c)
endif()

if(EVENT__HAVE_DEVPOLL)
    list(APPEND SRC_CORE devpoll.c)
endif()

if(EVENT__HAVE_EPOLL)
    list(APPEND SRC_CORE epoll.c)
endif()

if(EVENT__HAVE_EVENT_PORTS)
    list(APPEND SRC_CORE evport.c)
endif()

find_package(OpenSSL REQUIRED)

set(EVENT__HAVE_OPENSSL 1)

message(STATUS "OpenSSL include: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "OpenSSL lib: ${OPENSSL_LIBRARIES}")

include_directories(${OPENSSL_INCLUDE_DIR})

list(APPEND SRC_OPENSSL bufferevent_openssl.c)
list(APPEND HDR_PUBLIC include/event2/bufferevent_ssl.h)
list(APPEND LIB_APPS ${OPENSSL_LIBRARIES})

if (NOT EVENT__DISABLE_THREAD_SUPPORT)
    if (WIN32)
        list(APPEND SRC_CORE evthread_win32.c)
    else()
        find_package(Threads REQUIRED)
        if (NOT CMAKE_USE_PTHREADS_INIT)
            message(FATAL_ERROR
                    "Failed to find Pthreads, set EVENT__DISABLE_THREAD_SUPPORT to disable")
        endif()

        set(EVENT__HAVE_PTHREADS 1)
        list(APPEND LIB_APPS ${CMAKE_THREAD_LIBS_INIT})
    endif()
endif()

if (NOT EVENT__DISABLE_TESTS)
    # Zlib is only used for testing.
    find_package(ZLIB)

    if (ZLIB_LIBRARY AND ZLIB_INCLUDE_DIR)
        include_directories(${ZLIB_INCLUDE_DIRS})

        set(EVENT__HAVE_LIBZ 1)
        list(APPEND LIB_APPS ${ZLIB_LIBRARIES})
    endif()
endif()

set(SRC_EXTRA
    event_tagging.c
    http.c
    evdns.c
    evrpc.c)

add_definitions(-DHAVE_CONFIG_H)

# We use BEFORE here so we don't accidentally look in system directories
# first for some previous versions of the headers that are installed.
include_directories(BEFORE ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/compat
    ${PROJECT_SOURCE_DIR}/include)

if(WIN32)
    list(APPEND SRC_CORE
        buffer_iocp.c
        bufferevent_async.c
        event_iocp.c
        win32select.c)

    list(APPEND HDR_PRIVATE WIN32-Code/getopt.h)

    set(EVENT__DNS_USE_FTIME_FOR_ID 1)
    set(LIB_PLATFORM ws2_32)
    add_definitions(
            -D_CRT_SECURE_NO_WARNINGS
            -D_CRT_NONSTDC_NO_DEPRECATE)

    include_directories(./WIN32-Code)
endif()

if (SOLARIS)
    list(APPEND LIB_PLATFORM socket nsl)
endif()

source_group("Headers Private"  FILES ${HDR_PRIVATE})
source_group("Header Compat"    FILES ${HDR_COMPAT})
source_group("Headers Public"   FILES ${HDR_PUBLIC})
source_group("Source Core"      FILES ${SRC_CORE})
source_group("Source Extra"     FILES ${SRC_EXTRA})

# Generate the configure headers.
# (Place them in the build dir so we don't polute the source tree with generated files).
include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR}/include)

if ((CMAKE_COMPILER_IS_GNUCC) OR
    ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang") OR
    ("${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang"))
    set(EVENT_SHARED_FLAGS -fvisibility=hidden)
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "SunPro")
    set(EVENT_SHARED_FLAGS -xldscope=hidden)
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/event-config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/include/event2/event-config.h
        NEWLINE_STYLE UNIX)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/evconfig-private.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/include/evconfig-private.h)

#
# Create the libraries.
#
include(AddEventLibrary)
add_event_library(event_core SOURCES ${SRC_CORE})
add_event_library(event_extra
    LIBRARIES event_core_static
    SOURCES ${SRC_EXTRA})
add_event_library(event_openssl 
    LIBRARIES event_openssl_static
    SOURCES ${SRC_OPENSSL})

# library exists for historical reasons; it contains the contents of
# both libevent_core and libevent_extra. You shouldn’t use it; it may
# go away in a future version of Libevent.
add_event_library(event SOURCES ${SRC_CORE} ${SRC_EXTRA} ${SRC_OPENSSL})

set(WIN32_GETOPT)
if (WIN32)
    list(APPEND WIN32_GETOPT
         WIN32-Code/getopt.c
         WIN32-Code/getopt_long.c)
endif()

#
# Samples.
#
macro(add_sample_prog ssl name)
    add_executable(${name} ${ARGN})

    target_link_libraries(${name}
                          event_extra_static
                          event_core_static
                          event_openssl_static
                          ${LIB_APPS}
                          ${LIB_PLATFORM})
    add_dependencies(${name}
                     event_core_static
                     event_extra_static
                     event_openssl_static)
endmacro()
if (NOT EVENT__DISABLE_SAMPLES)
    set(SAMPLES
        event-read-fifo
        hello-world
        signal-test
        http-server
        http-connect
        time-test)

    foreach(SAMPLE ${SAMPLES})
        add_sample_prog(OFF ${SAMPLE} sample/${SAMPLE}.c)
    endforeach()

    add_sample_prog(ON https-client
                    sample/https-client.c
                    sample/openssl_hostname_validation.c
                    sample/hostcheck.c)
    add_sample_prog(ON le-proxy
                    sample/le-proxy.c)

    add_sample_prog(OFF dns-example sample/dns-example.c ${WIN32_GETOPT})
endif()

#
# Benchmarks
#
macro(add_bench_prog prog)
    add_executable(${prog} ${ARGN})
    target_link_libraries(${prog}
                          event_extra_static
                          event_core_static
                          ${LIB_APPS}
                          ${LIB_PLATFORM})
    add_dependencies(${prog}
                     event_extra_static
                     event_core_static)
endmacro()
if (NOT EVENT__DISABLE_BENCHMARK)
    foreach (BENCHMARK bench_http bench_httpclient)
        add_bench_prog(${BENCHMARK} test/${BENCHMARK}.c)
    endforeach()

    add_bench_prog(bench test/bench.c ${WIN32_GETOPT})
    add_bench_prog(bench_cascade test/bench_cascade.c ${WIN32_GETOPT})
endif()

#
# Tests
#
#
# Installation preparation.
#

if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR cmake)
else()
  set(DEF_INSTALL_CMAKE_DIR lib/cmake/libevent)
endif()

set(EVENT_INSTALL_CMAKE_DIR
    "${CMAKE_INSTALL_PREFIX}/${DEF_INSTALL_CMAKE_DIR}"
    CACHE PATH "Installation directory for CMake files")

export(PACKAGE libevent)

# Generate the config file for the build-tree.
set(EVENT__INCLUDE_DIRS
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_BINARY_DIR}/include")

set(LIBEVENT_INCLUDE_DIRS
    ${EVENT__INCLUDE_DIRS}
    CACHE PATH "Libevent include directories")

configure_file(${PROJECT_SOURCE_DIR}/cmake/LibeventConfigBuildTree.cmake.in
               ${PROJECT_BINARY_DIR}/LibeventConfig.cmake
               @ONLY)

# Generate the config file for the installation tree.
# Calculate the relative directory from the Cmake dir.
file(RELATIVE_PATH
     REL_INCLUDE_DIR
     "${EVENT_INSTALL_CMAKE_DIR}"
     "${CMAKE_INSTALL_PREFIX}/include")

# Note the EVENT_CMAKE_DIR is defined in LibeventConfig.cmake.in,
# we escape it here so it's evaluated when it is included instead
# so that the include dirs are givenrelative to where the
# config file is located.
set(EVENT__INCLUDE_DIRS "\${EVENT_CMAKE_DIR}/${REL_INCLUDE_DIR}")

configure_file(${PROJECT_SOURCE_DIR}/cmake/LibeventConfig.cmake.in
               ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/LibeventConfig.cmake
               @ONLY)

# Generate version info for both build-tree and install-tree.
configure_file(${PROJECT_SOURCE_DIR}/cmake/LibeventConfigVersion.cmake.in
               ${PROJECT_BINARY_DIR}/LibeventConfigVersion.cmake
               @ONLY)

# Install compat headers
install(FILES ${HDR_COMPAT}
        DESTINATION "include"
        COMPONENT dev)

# Install the configs.
install(FILES
        ${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/LibeventConfig.cmake
        ${PROJECT_BINARY_DIR}/LibeventConfigVersion.cmake
        DESTINATION "${EVENT_INSTALL_CMAKE_DIR}"
        COMPONENT dev)

message(STATUS "")
message(STATUS "        ---( Libevent " ${EVENT_VERSION} " )---")
message(STATUS "")
message(STATUS "Available event backends: ${BACKENDS}")
message(STATUS "CMAKE_BINARY_DIR:         " ${CMAKE_BINARY_DIR})
message(STATUS "CMAKE_CURRENT_BINARY_DIR: " ${CMAKE_CURRENT_BINARY_DIR})
message(STATUS "CMAKE_SOURCE_DIR:         " ${CMAKE_SOURCE_DIR})
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "PROJECT_BINARY_DIR:       " ${PROJECT_BINARY_DIR})
message(STATUS "PROJECT_SOURCE_DIR:       " ${PROJECT_SOURCE_DIR})
message(STATUS "CMAKE_MODULE_PATH:        " ${CMAKE_MODULE_PATH})
message(STATUS "CMAKE_COMMAND:            " ${CMAKE_COMMAND})
message(STATUS "CMAKE_ROOT:               " ${CMAKE_ROOT} )
message(STATUS "CMAKE_SYSTEM:             " ${CMAKE_SYSTEM} )
message(STATUS "CMAKE_SYSTEM_NAME:        " ${CMAKE_SYSTEM_NAME} )
message(STATUS "CMAKE_SYSTEM_VERSION:     " ${CMAKE_SYSTEM_VERSION} )
message(STATUS "CMAKE_SYSTEM_PROCESSOR:   " ${CMAKE_SYSTEM_PROCESSOR} )
message(STATUS "CMAKE_SKIP_RPATH:         " ${CMAKE_SKIP_RPATH} )
message(STATUS "CMAKE_VERBOSE_MAKEFILE:   " ${CMAKE_VERBOSE_MAKEFILE} )
message(STATUS "CMAKE_C_FLAGS:            " ${CMAKE_C_FLAGS} )
message(STATUS "CMAKE_BUILD_TYPE:         " ${CMAKE_BUILD_TYPE} )
message(STATUS "CMAKE_C_COMPILER:         " ${CMAKE_C_COMPILER} )
message(STATUS "CMAKE_AR:                 " ${CMAKE_AR} )
message(STATUS "CMAKE_RANLIB:             " ${CMAKE_RANLIB} )
message(STATUS "")

