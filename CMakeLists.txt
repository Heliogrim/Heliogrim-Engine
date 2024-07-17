cmake_minimum_required(VERSION 3.27.7)

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

# Project Declaration
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY CMAKE_FOLDER "External")
set(CMAKE_FOLDER "External")
set(IDE_FOLDER "Heliogrim")

project("Heliogrim" VERSION 1.0)

# CMake Tools
include(cmake/Architecture.cmake)
include(cmake/BuildType.cmake)
include(cmake/Tools.cmake)
include(cmake/Managed.cmake)
include(cmake/HealthCheck.cmake)
include(cmake/LinkTimeOpt.cmake)
include(cmake/Sanitizers.cmake)
include(cmake/MakeExecutableProject.cmake)
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
include(Options.cmake)

# Project Directories
## Deployment / Distribution directories will be configured within make library/test functions

# Tools
enable_cppcheck(OFF)
enable_clang_tidy(OFF)
enable_deploy(ON)

# Compile Options
include(cmake/PreprocDefinitions.cmake)
include(cmake/CompileOptions.cmake)

# Handle external dependencies
get_vendor_path(project_vendor_path)
file(GLOB externals CONFIGURE_DEPENDS LIST_DIRECTORIES true "${project_vendor_path}/*")
foreach (external ${externals})
	add_subdirectory("${external}")
endforeach ()
finalize_managed_libraries()

# Modules
get_projects_path(projects_directory)
list_directories("projects" "${projects_directory}")

set(tests ${projects})
list(FILTER projects EXCLUDE REGEX "(\\.Test)")
list(FILTER tests INCLUDE REGEX "\\.Test")

foreach (lib IN LISTS projects tests)
	add_subdirectory(${lib})
endforeach ()
