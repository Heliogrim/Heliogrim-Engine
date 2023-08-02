cmake_minimum_required(VERSION 3.24)

message(STATUS "------------------------------------------------------------------")
message(STATUS "                                                                  ")
message(STATUS "              __  __     ___                  _                   ")
message(STATUS "             / / / /__  / (_)___  ____ ______(_)___ ___           ")
message(STATUS "            / /_/ / _ \\/ / / __ \\/ __ `/ ___/ / __ `__ \\          ")
message(STATUS "           / __  /  __/ / / /_/ / /_/ / /  / / / / / / /          ")
message(STATUS "          /_/ /_/\\___/_/_/\\____/\\__, /_/  /_/_/ /_/ /_/           ")
message(STATUS "                               /____/                             ")
message(STATUS "                                                                  ")
message(STATUS "------------------------------------------------------------------")
message(STATUS "| License: Private                                               |")
message(STATUS "| Url: https://git.neogc.net/heliogrim-workgroup/engine          |")
message(STATUS "------------------------------------------------------------------")

# CMake Tools
include(cmake/Tools.cmake)
include(cmake/HealthCheck.cmake)
include(cmake/MakeLibraryProject.cmake)
include(cmake/MakeTestProject.cmake)
include(cmake/DeployDynLibs.cmake)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Profile")

# Project Options

include(cmake/Dependencies.cmake)

# Project Declaration
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(IDE_FOLDER "")

project("Heliogrim" VERSION 1.0)

# Project Directories
## Deployment / Distribution directories will be configured within make library/test functions

# Tools
enable_cppcheck(OFF)
enable_clang_tidy(OFF)
enable_deploy(ON)

# Compile Options
include(cmake/PreprocDefinitions.cmake)
include(cmake/CompileOptions.cmake)

# Modules
set(libraries "")
get_projects_path(projects_directory)
list_directories("libraries" "${projects_directory}")

set(tests ${libraries})

list(FILTER libraries EXCLUDE REGEX "\\.Test")
list(FILTER libraries EXCLUDE REGEX "(\\.Main)|(Main$)")
list(FILTER tests INCLUDE REGEX "\\.Test")

foreach (lib IN LISTS libraries tests)
    message(STATUS "${lib}")
    add_subdirectory(${lib})
endforeach ()

add_subdirectory("${projects_directory}/Heliogrim.Main")

add_subdirectory("${projects_directory}/Editor.Main")
add_subdirectory("${projects_directory}/Game.Main")
