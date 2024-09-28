#pragma once

#include <Engine.Common/Types.hpp>

namespace hg::engine::storage {
	/**/

	enum class MemoryProperty {
		eUndefined = 0x0,
		eProtected = 0x7
	};

	using MemoryRequestSize = u64;
}
