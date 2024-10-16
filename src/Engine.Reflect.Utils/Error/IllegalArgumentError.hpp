#pragma once

#include <stdexcept>

namespace hg {
	class IllegalArgumentError final : public std::runtime_error {
		using std::runtime_error::runtime_error;
	};
}
