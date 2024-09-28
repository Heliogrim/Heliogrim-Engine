# Dependency Resolving

macro(set_dep_policy)
	cmake_policy(SET CMP0048 NEW)
	set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)

	cmake_policy(SET CMP0077 NEW)
	set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

	# Modules FindPython3, FindPython2 and FindPython use LOCATION for lookup strategy.
	# https://cmake.org/cmake/help/latest/policy/CMP0094.html
	cmake_policy(SET CMP0094 NEW)
	set(CMAKE_POLICY_DEFAULT_CMP0094 NEW)

	if (POLICY CMP0126)
		cmake_policy(SET CMP0126 NEW)
		set(CMAKE_POLICY_DEFAULT_CMP0216 NEW)
	endif ()

	if (POLICY CMP0135)
		cmake_policy(SET CMP0135 NEW)
		set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)
	endif ()

	if (POLICY CMP0148)
		cmake_policy(SET CMP0148 NEW)
		set(CMAKE_POLICY_DEFAULT_CMP0148 NEW)
	endif ()
endmacro()
set_dep_policy()

# Configure
if (NOT DEFINED CMAKE_EP_GIT_REMOTE_UPDATE_STRATEGY)
	set(CMAKE_EP_GIT_REMOTE_UPDATE_STRATEGY REBASE_CHECKOUT)
endif ()

# Dependencies

include(CTest)