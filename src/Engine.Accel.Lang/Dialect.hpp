#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel::lang {
	enum class Dialect : u8 {
		/* ASL source text; the only dialect (the name predates the SPIR-V backend, the value is hashed) */
		eAccelGlsl
	};
}
