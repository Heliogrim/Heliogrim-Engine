#pragma once

#include <Engine.Common/__macro.hpp>

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

/* The glm provider includes this header for its configuration and wraps glm itself; everyone else imports it. */
#if not defined(HG_MODULE_INTERFACE_UNIT)
import glm;
#endif
