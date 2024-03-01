# Dependency Resolving

macro(set_dep_policy)
	cmake_policy(SET CMP0077 NEW)
	set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

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


# Dependencies

include(CTest)

include(FindVulkan)

if (NOT ${Vulkan_FOUND})
	message(FATAL_ERROR "Vulkan not found. Please specify include directory and library path.")
endif ()