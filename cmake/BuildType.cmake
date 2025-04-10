
set(DEFAULT_BUILD_TYPE "RelWithDebInfo")

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose a build type." FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Profile" "Release" "RelWithDebInfo")
endif ()

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;Profile")

if (NOT CMAKE_EXE_LINKER_FLAGS_PROFILE)
	set(CMAKE_C_FLAGS_PROFILE ${CMAKE_C_FLAGS_RELWITHDEBINFO})
	set(CMAKE_CXX_FLAGS_PROFILE ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
	set(CMAKE_EXE_LINKER_FLAGS_PROFILE ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO})
	set(CMAKE_SHARED_LINKER_FLAGS_PROFILE ${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO})
endif ()