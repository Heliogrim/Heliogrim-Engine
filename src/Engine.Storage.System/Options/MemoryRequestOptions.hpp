#pragma once

#include <new>
#include <stdexcept>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Variant.hpp>

namespace hg::engine::storage {
	struct RequestMemoryOptions {
		streamsize requestSize;
	};

	using request_memory_error = Variant<std::bad_alloc>;
	using request_memory_located_error = Variant<std::bad_alloc, std::runtime_error>;
}
