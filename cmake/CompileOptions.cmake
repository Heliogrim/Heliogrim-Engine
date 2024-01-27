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
        CXX_EXTENSIONS Off
        )


#
# Include directories
#
get_library_path(lib_dir)
set(DEFAULT_INCLUDE_DIRECTORIES
    "${Vulkan_INCLUDE_DIR}"
    "${lib_dir}/ankerl"
    "${lib_dir}/assimp"
    "${lib_dir}/dr_flac"
    "${lib_dir}/dr_wav"
    "${lib_dir}/freetype2"
    "${lib_dir}/gli"
    "${lib_dir}/glm"
    "${lib_dir}/glslang"
    "${lib_dir}/ryml"
    "${lib_dir}/SDL2"
    "${lib_dir}/spdlog"
    "${lib_dir}/tsl"
    "${lib_dir}/tl"
    "${lib_dir}/vorbis")

set(TEST_INCLUDE_DIRECTORIES
    "${lib_dir}/gtest")

#
# Libraries
#
set(DEFAULT_LIBRARIES
    "${Vulkan_LIBRARY}"
    "${lib_dir}/bin/SDL2/${CMAKE_BUILD_TYPE}/SDL2.lib"
    "${lib_dir}/bin/vorbis/${CMAKE_BUILD_TYPE}/ogg.lib"
    "${lib_dir}/bin/vorbis/${CMAKE_BUILD_TYPE}/vorbis.lib"
    "${lib_dir}/bin/vorbis/${CMAKE_BUILD_TYPE}/vorbisenc.lib"
    "${lib_dir}/bin/vorbis/${CMAKE_BUILD_TYPE}/vorbisfile.lib")

set(TEST_LIBRARIES
    "${lib_dir}/bin/gtest/${CMAKE_BUILD_TYPE}/gtestd.lib"
    "${lib_dir}/bin/gtest/${CMAKE_BUILD_TYPE}/gtest_maind.lib"
    "${lib_dir}/bin/gtest/${CMAKE_BUILD_TYPE}/gmockd.lib"
    "${lib_dir}/bin/gtest/${CMAKE_BUILD_TYPE}/gmock_maind.lib")

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    set(DEFAULT_LIBRARIES
        ${DEFAULT_LIBRARIES}
        "${lib_dir}/bin/freetype2/${CMAKE_BUILD_TYPE}/freetyped.lib"
        "${lib_dir}/bin/assimp/${CMAKE_BUILD_TYPE}/assimp-vc142-mtd.lib"
        "${lib_dir}/bin/spdlog/${CMAKE_BUILD_TYPE}/spdlogd.lib"
        #
        "${lib_dir}/bin/glslang/${CMAKE_BUILD_TYPE}/GenericCodeGend.lib"
        "${lib_dir}/bin/glslang/${CMAKE_BUILD_TYPE}/glslang-default-resource-limitsd.lib"
        "${lib_dir}/bin/glslang/${CMAKE_BUILD_TYPE}/glslangd.lib"
        "${lib_dir}/bin/glslang/${CMAKE_BUILD_TYPE}/HLSLd.lib"
        "${lib_dir}/bin/glslang/${CMAKE_BUILD_TYPE}/MachineIndependentd.lib"
        "${lib_dir}/bin/glslang/${CMAKE_BUILD_TYPE}/OGLCompilerd.lib"
        "${lib_dir}/bin/glslang/${CMAKE_BUILD_TYPE}/OSDependentd.lib"
        "${lib_dir}/bin/glslang/${CMAKE_BUILD_TYPE}/SPIRVd.lib"
        "${lib_dir}/bin/glslang/${CMAKE_BUILD_TYPE}/SPVRemapperd.lib")

else ()
    set(DEFAULT_LIBRARIES
        ${DEFAULT_LIBRARIES}
		"${lib_dir}/bin/freetype2/${CMAKE_BUILD_TYPE}/freetype.lib"
		"${lib_dir}/bin/assimp/${CMAKE_BUILD_TYPE}/assimp-vc142-mt.lib"
        "${lib_dir}/bin/spdlog/${CMAKE_BUILD_TYPE}/spdlog.lib"
		#
		"${lib_dir}/bin/glslang/${CMAKE_BUILD_TYPE}/GenericCodeGen.lib"
		"${lib_dir}/bin/glslang/${CMAKE_BUILD_TYPE}/glslang-default-resource-limits.lib"
		"${lib_dir}/bin/glslang/${CMAKE_BUILD_TYPE}/glslang.lib"
		"${lib_dir}/bin/glslang/${CMAKE_BUILD_TYPE}/HLSL.lib"
		"${lib_dir}/bin/glslang/${CMAKE_BUILD_TYPE}/MachineIndependent.lib"
		"${lib_dir}/bin/glslang/${CMAKE_BUILD_TYPE}/OGLCompiler.lib"
		"${lib_dir}/bin/glslang/${CMAKE_BUILD_TYPE}/OSDependent.lib"
		"${lib_dir}/bin/glslang/${CMAKE_BUILD_TYPE}/SPIRV.lib"
		"${lib_dir}/bin/glslang/${CMAKE_BUILD_TYPE}/SPVRemapper.lib")
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

#
# Compile launch options
#

find_program(CCACHE_FOUND ccache)
if (CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
    set(CMAKE_C_COMPILER_LAUNCHER ccache)
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

            -Wreorder
            -Wignored-qualifiers
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

            $<$<CXX_COMPILER_ID:GNU>:
            -Wmaybe-uninitialized

            $<$<VERSION_GREATER:$<CXX_COMPILER_VERSION>,4.8>:
            -Wpedantic

            -Wreturn-local-addr
            >
            >

            $<$<CXX_COMPILER_ID:Clang>:
            -Wpedantic

            -Wno-c++98-compat-pedantic
			-Wno-c++11-compat-pedantic
			-Wno-c++14-compat-pedantic
			-Wno-c++17-compat-pedantic

            # -Wreturn-stack-address # gives false positives
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
