#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::storage {
	struct RequestMemoryOptions {
		streamsize requestSize;
	};

	using request_memory_error = Variant<std::bad_alloc>;
	using request_memory_located_error = Variant<std::bad_alloc, std::runtime_error>;
}
