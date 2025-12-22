cmake_minimum_required(VERSION 3.16)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
project(SimpleJSON LANGUAGES C)
include(FetchContent)

set(RING_ROOT_DEFAULT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
if(DEFINED ENV{RING})
    set(RING_ROOT_DEFAULT "$ENV{RING}")
endif()
set(RING_ROOT "${RING_ROOT_DEFAULT}" CACHE PATH "Path to the Ring project root directory.")
message(STATUS "Using Ring from: ${RING_ROOT}")

if(NOT TARGET Ring::Ring)
    add_library(Ring::Ring UNKNOWN IMPORTED)
    find_path(RING_INCLUDE_DIR ring.h PATHS "${RING_ROOT}/language/include")
    find_library(RING_LIBRARY_PATH NAMES ring PATHS "${RING_ROOT}/lib")

    if(NOT RING_INCLUDE_DIR OR NOT RING_LIBRARY_PATH)
        message(FATAL_ERROR "Could not find Ring headers or library in ${RING_ROOT}. "
                            "Please set the RING_ROOT cache variable correctly.")
    endif()

    set_target_properties(Ring::Ring PROPERTIES
        IMPORTED_LOCATION "${RING_LIBRARY_PATH}"
        INTERFACE_INCLUDE_DIRECTORIES "${RING_INCLUDE_DIR}"
    )
endif()

# Set paths
set(RING_INCLUDE "${RING_ROOT}/language/include")
set(RING_LIB "${RING_ROOT}/lib")
set(RING_BIN "${RING_ROOT}/bin")

# Determine OS and Architecture specific paths
string(TOLOWER "${CMAKE_SYSTEM_NAME}" OS_DIR)
if(OS_DIR STREQUAL "darwin")
    set(OS_DIR "macos")
endif()

string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" ARCH_DIR_RAW)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND DEFINED CMAKE_GENERATOR_PLATFORM)
    if(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
        set(ARCH_DIR_RAW "i386")
    elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "x64")
        set(ARCH_DIR_RAW "amd64")
    elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "ARM64")
        set(ARCH_DIR_RAW "arm64")
    else()
        message(WARNING "Unsupported CMAKE_GENERATOR_PLATFORM: ${CMAKE_GENERATOR_PLATFORM}, falling back to CMAKE_SYSTEM_PROCESSOR")
    endif()
endif()

if(NOT DEFINED ARCH_DIR)
    if(ARCH_DIR_RAW MATCHES "x86_64|amd64")
        set(ARCH_DIR "amd64")
    elseif(ARCH_DIR_RAW MATCHES "aarch64|arm64")
        set(ARCH_DIR "arm64")
    elseif(ARCH_DIR_RAW MATCHES "riscv64")
        set(ARCH_DIR "riscv64")
    elseif(ARCH_DIR_RAW MATCHES "i386|i686|x8x")
        set(ARCH_DIR "i386")
    else()
        set(ARCH_DIR "${ARCH_DIR_RAW}")
        message(WARNING "Unsupported architecture: ${ARCH_DIR_RAW}. Using as directory name.")
    endif()
endif()

# Detect musl libc
option(MUSL "Build for musl libc" OFF)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Auto-detect musl if not explicitly set
    if(NOT MUSL)
        execute_process(
            COMMAND ldd --version
            OUTPUT_VARIABLE LDD_OUTPUT
            ERROR_VARIABLE LDD_OUTPUT
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(LDD_OUTPUT MATCHES "musl")
            set(MUSL ON)
            message(STATUS "Detected musl libc")
        endif()
    endif()
endif()

# Set the destination directory for the built library
if(MUSL AND OS_DIR STREQUAL "linux")
    set(LIB_DEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/${OS_DIR}/musl/${ARCH_DIR}")
else()
    set(LIB_DEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/${OS_DIR}/${ARCH_DIR}")
endif()

# Configure Jansson build options
set(JANSSON_BUILD_SHARED_LIBS OFF CACHE BOOL "Build Jansson as a static library")
set(USE_URANDOM ON CACHE BOOL "Use urandom for random number generation")
set(USE_WINDOWS_CRYPTOAPI ON CACHE BOOL "Use Windows CryptoAPI for random number generation")
set(USE_DTOA ON CACHE BOOL "Use dtoa for double conversion")
set(JANSSON_STATIC_CRT OFF CACHE BOOL "Do not use static CRT on Windows")
set(JANSSON_EXAMPLES OFF CACHE BOOL "Do not build Jansson examples")
set(JANSSON_COVERAGE OFF CACHE BOOL "Do not build with coverage")
set(JANSSON_BUILD_DOCS OFF CACHE BOOL "Do not build Jansson documentation")
set(JANSSON_WITHOUT_TESTS ON CACHE BOOL "Do not build Jansson tests")

# Add jansson as a subdirectory to build it
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/jansson/CMakeLists.txt")
    message(STATUS "jansson source not found, fetching from GitHub...")
    FetchContent_Declare(
        jansson
        GIT_REPOSITORY https://github.com/akheron/jansson.git
        GIT_TAG        v2.14.1
    )
    FetchContent_MakeAvailable(jansson)
else()
    message(STATUS "jansson found locally, adding as subdirectory.")
    add_subdirectory(src/jansson)
endif()

# Create the Ring SimpleJSON extension shared library
add_library(ring_simplejson SHARED
	${CMAKE_CURRENT_SOURCE_DIR}/src/ring_simplejson.c
)

# Include directories
target_include_directories(ring_simplejson PRIVATE
	${RING_INCLUDE}
	${jansson_SOURCE_DIR}/src
	${jansson_BINARY_DIR}/include
)

# Link libraries
target_link_libraries(ring_simplejson PRIVATE
	Ring::Ring
	jansson
)

target_compile_options(ring_simplejson PRIVATE
    $<$<CONFIG:Release,RelWithDebInfo,MinSizeRel>:
        $<$<C_COMPILER_ID:MSVC>:/O2>
        $<$<NOT:$<C_COMPILER_ID:MSVC>>:-O3>
        -DNDEBUG
    >
)

# Set target properties for output name and prefix
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set_target_properties(ring_simplejson PROPERTIES
		OUTPUT_NAME "ring_simplejson"
	)
else()
	set_target_properties(ring_simplejson PROPERTIES
		PREFIX "lib"
		OUTPUT_NAME "ring_simplejson"
	)
endif()

# Move built library to lib directory
add_custom_command(
	TARGET ring_simplejson
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${LIB_DEST_DIR}
	COMMAND ${CMAKE_COMMAND} -E rename $<TARGET_FILE:ring_simplejson> ${LIB_DEST_DIR}/$<TARGET_FILE_NAME:ring_simplejson>
	COMMENT "Moving built library to ${LIB_DEST_DIR}"
	VERBATIM
)

# Copy to bin directory on Windows
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	add_custom_command(
		TARGET ring_simplejson
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory ${RING_BIN}
		COMMAND ${CMAKE_COMMAND} -E copy ${LIB_DEST_DIR}/$<TARGET_FILE_NAME:ring_simplejson> ${RING_BIN}/$<TARGET_FILE_NAME:ring_simplejson>
		COMMENT "Copying built library to bin directory"
		VERBATIM
	)
endif()

install(TARGETS ring_simplejson
    LIBRARY DESTINATION "${RING_ROOT}/lib"
)

message(STATUS "Ring SimpleJSON Extension Configuration:")
message(STATUS "  - Ring Include Dir: ${RING_INCLUDE_DIR}")
message(STATUS "  - Ring Library Path: ${RING_LIBRARY_PATH}")