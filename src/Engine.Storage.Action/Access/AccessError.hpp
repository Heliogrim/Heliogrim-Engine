#pragma once

#include <stdexcept>

namespace hg::engine::storage {
	class AccessError final : public std::runtime_error {
		using std::runtime_error::runtime_error;
	};
}
