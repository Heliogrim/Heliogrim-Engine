include_guard(GLOBAL)

set_property(GLOBAL PROPERTY link_time_optimization_available FALSE)
set_property(GLOBAL PROPERTY link_time_optimization_enabled FALSE)
set_property(GLOBAL PROPERTY link_time_optimization_required FALSE)

# Translate options

include(CheckIPOSupported)

option(LTO "Switch to enable link-time-optimization." ON)
option(LTO_REQUIRED "Switch to require or fail link-time-optimization." OFF)

if (LTO)
    set_property(GLOBAL PROPERTY link_time_optimization_enabled TRUE)
else ()
    set_property(GLOBAL PROPERTY link_time_optimization_enabled FALSE)
endif ()

if (LTO_REQUIRED)
    set_property(GLOBAL PROPERTY link_time_optimization_required TRUE)
else ()
    set_property(GLOBAL PROPERTY link_time_optimization_required FALSE)
endif ()

# Check availability
check_ipo_supported(RESULT result OUTPUT report)
if (result)
    set_property(GLOBAL PROPERTY link_time_optimization_available TRUE)
else ()
    if (link_time_optimization_required)
        message(FATAL_ERROR "Link-Time-Optimization required, but ipo is not supported.")
    elseif (link_time_optimization_enabled)
        message(WARNING "Link-Time-Optimization enabled, but ipo is not supported.")
    endif ()
endif ()

# Helper macro
macro(set_target_lto target)
    get_property(__lto_available GLOBAL PROPERTY link_time_optimization_available)
    get_property(__lto_enabled GLOBAL PROPERTY link_time_optimization_enabled)
    if (__lto_available AND __lto_enabled)
        set_property(TARGET ${target} PROPERTY INTERPROCEDURAL_OPTIMIZATION (__lto_available AND __lto_enabled))
        message(VERBOSE "Enabling link-time-optimization for target `${target}`")
    endif ()
endmacro()