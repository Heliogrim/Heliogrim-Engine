#
# Platform and architecture
#

set(__vk_platforms "")

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC" OR "${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
	add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
	list(APPEND __vk_platforms VK_USE_PLATFORM_WIN32_KHR)
endif ()

if (
		"${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" OR
		"${CMAKE_SYSTEM_NAME}" MATCHES "Linux" OR
		"${CMAKE_SYSTEM_NAME}" MATCHES "DragonFly" OR
		"${CMAKE_SYSTEM_NAME}" MATCHES "FreeBSD"
)
	if (DEFINED HELIOGRIM_PLATFORM_WAYLAND AND HELIOGRIM_PLATFORM_WAYLAND)
		add_definitions(-DVK_USE_PLATFORM_WAYLAND_KHR)
		list(APPEND __vk_platforms VK_USE_PLATFORM_WAYLAND_KHR)
	endif ()

	if (DEFINED HELIOGRIM_PLATFORM_X11 AND HELIOGRIM_PLATFORM_X11)
		list(APPEND __vk_platforms VK_USE_PLATFORM_XCB_KHR VK_USE_PLATFORM_XLIB_KHR)
		add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
		add_definitions(-DVK_USE_PLATFORM_XLIB_KHR)
	endif ()
endif ()

list(LENGTH __vk_platforms __vk_platform_count)
if (${__vk_platform_count} LESS_EQUAL 0)
	message(FATAL_ERROR "Unable to determine vulkan platform to use as none is selected.")
endif ()