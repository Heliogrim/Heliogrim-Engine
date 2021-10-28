cmake_minimum_required(VERSION 3.21)

# CMake Tools
include(cmake/Tools.cmake)
include(cmake/HealthCheck.cmake)
include(cmake/MakeLibraryProject.cmake)
include(cmake/MakeTestProject.cmake)
include(cmake/DeployDynLibs.cmake)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Profile")

# Meta
set(META_PROJECT_NAME "Ember")
set(META_PROJECT_ROOT_DIR "${CMAKE_SOURCE_DIR}")
set(META_PROJECT_SUB_DIR "${META_PROJECT_ROOT_DIR}/projects")
set(META_PROJECT_DIST_DIR "${META_PROJECT_ROOT_DIR}/dist")
set(META_PROJECT_SRC_DIR "${META_PROJECT_ROOT_DIR}/src")
set(META_PROJECT_LIB_DIR "${META_PROJECT_ROOT_DIR}/library")

# Project Options

# Project Declaration
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(IDE_FOLDER "")

project(${META_PROJECT_NAME} VERSION 1.0)

# Project Directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${META_PROJECT_DIST_DIR}/Debug")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${META_PROJECT_DIST_DIR}/Debug")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${META_PROJECT_DIST_DIR}/Debug")
set(CMAKE_PDB_OUTPUT_DIRECTORY_DEBUG "${META_PROJECT_DIST_DIR}/Debug")
set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY_DEBUG "${META_PROJECT_DIST_DIR}/Debug")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${META_PROJECT_DIST_DIR}/Release")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${META_PROJECT_DIST_DIR}/Release")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${META_PROJECT_DIST_DIR}/Release")
set(CMAKE_PDB_OUTPUT_DIRECTORY_RELEASE "${META_PROJECT_DIST_DIR}/Release")
set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY_RELEASE "${META_PROJECT_DIST_DIR}/Release")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_PROFILE "${META_PROJECT_DIST_DIR}/Profile")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_PROFILE "${META_PROJECT_DIST_DIR}/Profile")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_PROFILE "${META_PROJECT_DIST_DIR}/Profile")
set(CMAKE_PDB_OUTPUT_DIRECTORY_PROFILE "${META_PROJECT_DIST_DIR}/Profile")
set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY_PROFILE "${META_PROJECT_DIST_DIR}/Profile")

# Tools
enable_cppcheck(OFF)
enable_clang_tidy(OFF)
enable_deploy(ON)

# Compile Options
include(cmake/Dependencies.cmake)
include(cmake/PreprocDefinitions.cmake)
include(cmake/CompileOptions.cmake)

# Modules
set(libraries "")
list_directories("libraries" "${META_PROJECT_SUB_DIR}")

set(tests ${libraries})

list(FILTER libraries EXCLUDE REGEX "\\.Test")
list(FILTER libraries EXCLUDE REGEX "Game([^/]+)")
list(FILTER tests INCLUDE REGEX "\\.Test")

foreach (lib IN LISTS libraries tests)
    message(STATUS "${lib}")
    add_subdirectory(${lib})
endforeach ()

add_subdirectory("${META_PROJECT_SUB_DIR}/Game.Main")