#pragma once

#include <Engine.Common/GLMConfig.hpp>

/* The glm provider includes this header for its configuration and wraps glm itself; everyone else imports it. */
#if not defined(HG_MODULE_INTERFACE_UNIT)
import glm;
#endif
