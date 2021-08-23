cmake_minimum_required(VERSION 3.17.0)
project(Firefly VERSION 1.0.1)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

option(USE_64BIT_BUILD "USE_64BIT_BUILD" ON)
option(USE_STATIC_LINKING "USE_STATIC_LINKING" OFF)

if (USE_STATIC_LINKING)
    message("WARNING: Linking statically")
    set(FIREFLY_LINK_TYPE "static")
else ()
    message("WARNING: Linking dynamically")
    set(FIREFLY_LINK_TYPE "dynamic")
endif ()

message("GENERATOR USED: '${CMAKE_GENERATOR}'")
message("COMPILER USED: '${CMAKE_CXX_COMPILER_ID}'")

########
# Output
########

set(CMAKE_BINARY_DIR "${PROJECT_SOURCE_DIR}/dist/${CMAKE_BUILD_TYPE}")
set(CMAKE_BINARY_DIR_DEBUG "${PROJECT_SOURCE_DIR}/dist/Debug")
set(CMAKE_BINARY_DIR_RELEASE "${PROJECT_SOURCE_DIR}/dist/Release")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR_RELEASE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR_DEBUG})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR_RELEASE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR_DEBUG})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR_RELEASE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR_DEBUG})

###################
# Compiler & Linker
###################

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")

    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -g")

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

    string(REPLACE "/GR" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /GR- /Oi /Qpar /fp:precise")
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /Od /Ob0 /RTC1 /Oy-")
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD /O2 /Ob2 /Oy /Zc:inline")

    set (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /debug:fastlink")
    set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /incremental /OPT:REF")

endif()

set (CMAKE_CL_64 ${USE_64BIT_BUILD})

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE INTERNAL "cxx compiler flags")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}" CACHE INTERNAL "cxx debug compiler flags")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}" CACHE INTERNAL "cxx release compiler flags")

set (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}" CACHE INTERNAL "debug linker flags")
set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE}" CACHE INTERNAL "release linker flags")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

#########
# Sources
#########
set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
file(GLOB_RECURSE SOURCES_CPP ${SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE SOURCES_HPP ${SOURCE_DIR}/*.h ${SOURCE_DIR}/**.hpp)

############
# Executable
############
add_executable(Firefly ${SOURCE_HPP} ${SOURCES_CPP})

###########
# Libraries
###########

# Library - Assimp
include_directories(library/assimp/include)

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    set(ASSIMP_DIR  "${PROJECT_SOURCE_DIR}/library/bin/assimp/Debug")
    set(EXTERNAL_LIBRARIES ${EXTERNAL_LIBRARIES} ${ASSIMP_DIR}/assimp-vc142-mtd.lib)

elseif(CMAKE_BUILD_TYPE MATCHES "Release")
    set(ASSIMP_DIR  "${PROJECT_SOURCE_DIR}/library/bin/assimp/Release")
    set(EXTERNAL_LIBRARIES ${EXTERNAL_LIBRARIES} ${ASSIMP_DIR}/assimp-vc142-mt.lib)

endif ()

# Library - GLI
include_directories(library/gli)

# Library - GLM
include_directories(library/glm)

# Library - SDL2
include_directories($ENV{VULKAN_SDK}/Third-Party/Include)

set(SDL2_DIR "$ENV{VULKAN_SDK}/Third-Party/Bin")
set(EXTERNAL_LIBRARIES ${EXTERNAL_LIBRARIES} ${SDL2_DIR}/SDL2.lib)

# Library - VULKAN
include_directories($ENV{VULKAN_SDK}/Include)

set(VULKAN_DIR "$ENV{VULKAN_SDK}/Lib")
set(EXTERNAL_LIBRARIES ${EXTERNAL_LIBRARIES} ${VULKAN_DIR}/vulkan-1.lib)

# Library Linking
target_link_libraries(Firefly ${EXTERNAL_LIBRARIES})

##############
# Installation
##############

#install(TARGETS Firefly
#        CONFIGURATIONS Debug
#        DESTINATION ${PROJECT_SOURCE_DIR}/dist/Debug
#        )
#install(TARGETS Firefly
#        CONFIGURATIONS Release
#        DESTINATION ${PROJECT_SOURCE_DIR}/dist/Release
#        )