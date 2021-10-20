# Dependencies

include(FindVulkan)

if (NOT ${Vulkan_FOUND})
    message(FATAL_ERROR "Vulkan not found. Please specify include directory and library path.")
endif ()