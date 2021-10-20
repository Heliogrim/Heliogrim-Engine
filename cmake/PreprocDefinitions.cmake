#
# Platform and architecture
#

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" OR "${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
endif()