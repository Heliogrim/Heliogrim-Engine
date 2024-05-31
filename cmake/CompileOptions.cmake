#
# Platform and architecture setup
#

# Get upper case system name
string(TOUPPER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME_UPPER)

# Determine architecture (32/64 bit)
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(X64 ON)
else ()
    message(FATAL_ERROR "System with x64 is required to use this project.")
endif ()

#
# Project options
#

set(DEFAULT_PROJECT_OPTIONS
        C_STANDARD 17
        CXX_STANDARD 23
        LINKER_LANGUAGE "CXX"
        POSITION_INDEPENDENT_CODE ON
        CXX_VISIBILITY_PRESET "hidden"
        CXX_EXTENSIONS OFF
)


#
# Include directories
#
get_library_path(lib_dir)
set(DEFAULT_INCLUDE_DIRECTORIES
        "${Vulkan_INCLUDE_DIR}"
        "${lib_dir}/ankerl/include"
        "${lib_dir}/assimp/include"
        "${lib_dir}/dr_libs/include"
        "${lib_dir}/expected/include"
        "${lib_dir}/freetype2/include"
        "${lib_dir}/gli/include"
        "${lib_dir}/glm/include"
        "${lib_dir}/glslang/include"
        "${lib_dir}/ogg/include"
        "${lib_dir}/optional/include"
        "${lib_dir}/ryml/include"
        "${lib_dir}/sdl/include"
        "${lib_dir}/spdlog/include"
        "${lib_dir}/spng/include"
        "${lib_dir}/stb/include"
        "${lib_dir}/tinyexr/include"
        "${lib_dir}/vorbis/include"
        "${lib_dir}/zlib/include")

set(TEST_INCLUDE_DIRECTORIES
        "${lib_dir}/gtest/include")

#
# Libraries
#
set(DEFAULT_LIBRARIES
        # External Targets
        ${PROJECT_NAME}::External-assimp
        ${PROJECT_NAME}::External-freetype
        ${PROJECT_NAME}::External-ogg
        ${PROJECT_NAME}::External-sdl
        ${PROJECT_NAME}::External-spdlog
        ${PROJECT_NAME}::External-spng
        ${PROJECT_NAME}::External-vorbis
        ${PROJECT_NAME}::External-vorbisenc
        ${PROJECT_NAME}::External-vorbisfile
        ${PROJECT_NAME}::External-zlib
        ${PROJECT_NAME}::External-glslang
        ${PROJECT_NAME}::External-GenericCodeGen
        ${PROJECT_NAME}::External-glslang-default-resource-limits
        ${PROJECT_NAME}::External-MachineIndependent
        ${PROJECT_NAME}::External-OSDependent
        ${PROJECT_NAME}::External-SPIRV
        ${PROJECT_NAME}::External-SPVRemapper

        # External Libraries (Legacy)
        "${Vulkan_LIBRARY}")

# Append Sanitizer Options
if (SANITIZER_LINK_LIBS)
    list(APPEND DEFAULT_LIBRARIES ${SANITIZER_LINK_LIBS})
endif ()

set(TEST_LIBRARIES
        # External Targets
        ${PROJECT_NAME}::External-gtest
        ${PROJECT_NAME}::External-gtest_main
        ${PROJECT_NAME}::External-gmock
        ${PROJECT_NAME}::External-gmock_main)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    set(DEFAULT_LIBRARIES ${DEFAULT_LIBRARIES})
else ()
    set(DEFAULT_LIBRARIES ${DEFAULT_LIBRARIES})
endif ()

#
# Compile definitions
#
set(DEFAULT_COMPILE_DEFINITIONS
        SYSTEM_${SYSTEM_NAME_UPPER}
)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    set(DEFAULT_COMPILE_DEFINITIONS ${DEFAULT_COMPILE_DEFINITIONS}
            _Debug # Custom Debug Preprocessor Flag
    )
endif ()
if (${CMAKE_BUILD_TYPE} MATCHES "Profile")
    set(DEFAULT_COMPILE_DEFINITIONS ${DEFAULT_COMPILE_DEFINITIONS}
            _Profiling # Custom Debug Preprocessor Flag
    )
endif ()

# MSVC compiler options
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    set(DEFAULT_COMPILE_DEFINITIONS ${DEFAULT_COMPILE_DEFINITIONS}
            _SCL_SECURE_NO_WARNINGS  # Calling any one of the potentially unsafe methods in the Standard C++ Library
            _CRT_SECURE_NO_WARNINGS  # Calling any one of the potentially unsafe methods in the CRT Library
    )
endif ()

# Append Sanitizer Options
if (SANITIZER_COMPILE_DEFINITIONS)
    list(APPEND DEFAULT_COMPILE_DEFINITIONS ${SANITIZER_COMPILE_DEFINITIONS})
endif ()

#
# Compile launch options
#

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    # Prevent launch control of CCache with whitespace prone resources
    #set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    # CCache should not be used for linking, as is can make a mess
    #set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    message(STATUS "Found ccache.")
endif ()


#
# Compile options
#

set(DEFAULT_COMPILE_OPTIONS)

# MSVC compiler options
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    set(DEFAULT_COMPILE_OPTIONS ${DEFAULT_COMPILE_OPTIONS}
            PRIVATE
            /MP           # -> build with multiple processes
            # Warning: /W4 will currently crash the msvc compiler with ICE 1001
            /W3           # -> warning level 4
            # /WX         # -> treat warnings as errors

            # Compiler Shared Options
            /Oi
            /Gy
            /Qpar
            /GR-
            /Gd
            /GT         # -> Fiber safe thread local storage

            /Zc:wchar_t
            /Zc:inline
            /Zc:forScope
            /permissive-


            #$<$<CONFIG:Debug>:
            #/RTCc         # -> value is assigned to a smaller data type and results in a data loss
            #>
            $<$<CONFIG:Debug>:
            /RTC1
            /sdl
            /Oy-
            /Od
            /GS
            /JMC
            >

            $<$<CONFIG:Release>:
            /Gw           # -> whole program global optimization
            /GS-          # -> buffer security check: no
            /GL           # -> whole program optimization: enable link-time code generation (disables Zi)
            /GF           # -> enable string pooling
            >

            # No manual c++11 enable for MSVC as all supported MSVC versions for cmake-init have C++11 implicitly enabled (MSVC >=2013)

            PUBLIC
            /wd4251       # -> disable warning: 'identifier': class 'type' needs to have dll-interface to be used by clients of class 'type2'
            /wd4592       # -> disable warning: 'identifier': symbol will be dynamically initialized (implementation limitation)
            # /wd4201     # -> disable warning: nonstandard extension used: nameless struct/union (caused by GLM)
            # /wd4127     # -> disable warning: conditional expression is constant (caused by Qt)
    )
endif ()

# GCC and Clang compiler options
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(DEFAULT_COMPILE_OPTIONS ${DEFAULT_COMPILE_OPTIONS}
            PRIVATE

            -Wall
            -Wextra
            -Wunused
            -Weffc++

            -Wreorder
            -Wmissing-braces
            -Wreturn-type
            -Wswitch
            -Wswitch-default
            -Wuninitialized
            -Wmissing-field-initializers

            -Wno-c++98-compat
            -Wno-c++11-compat
            -Wno-c++14-compat
            -Wno-c++17-compat
            -Wno-c++20-compat

            -Wno-reserved-identifier
            -Wno-reserved-macro-identifier

            $<$<CXX_COMPILER_ID:GNU>:
            -Wmaybe-uninitialized

            $<$<VERSION_GREATER:$<CXX_COMPILER_VERSION>,4.8>:
            -Wpedantic
            -Wreturn-local-addr
            >
            >

            $<$<CXX_COMPILER_ID:Clang>:
            -Wpedantic
            -Wpedantic-macros
            -Wpedantic-core-features

            -Wno-c++98-compat-pedantic
            -Wno-c++11-compat-pedantic
            -Wno-c++14-compat-pedantic
            -Wno-c++17-compat-pedantic

            # -Wreturn-stack-address # gives false positives
            -Wno-gnu-anonymous-struct
            -Wno-nested-anon-types
            >

            $<$<BOOL:${OPTION_COVERAGE_ENABLED}>:
            -fprofile-arcs
            -ftest-coverage
            >

            PUBLIC
            $<$<PLATFORM_ID:Darwin>:
            -pthread
            >

            # Required for CMake < 3.1; should be removed if minimum required CMake version is raised.
            $<$<VERSION_LESS:${CMAKE_VERSION},3.1>:
            -std=c++23
            >
    )
endif ()

# Append Sanitizer Options
if (SANITIZER_COMPILE_OPTIONS)
    list(APPEND DEFAULT_COMPILE_OPTIONS ${SANITIZER_COMPILE_OPTIONS})
endif ()


#
# Linker options
#

set(DEFAULT_LINKER_OPTIONS)

# Use pthreads on mingw and linux
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" OR "${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    set(DEFAULT_LINKER_OPTIONS
            PUBLIC
            ${DEFAULT_LINKER_OPTIONS}
            -pthread
    )

    if (${OPTION_COVERAGE_ENABLED})
        set(DEFAULT_LINKER_OPTIONS
                PUBLIC
                ${DEFAULT_LINKER_OPTIONS}
                -fprofile-arcs
                -ftest-coverage
        )
    endif ()
endif ()
