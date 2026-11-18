#pragma once

#include <Engine.Common/Types.hpp>

namespace hg::engine::accel::lang {
	enum class Dialect : u8 {
		/* ASL source text; the only dialect (the name predates the SPIR-V backend, the value is hashed) */
		eAccelGlsl
	};
}
