#pragma once

#include <stdexcept>

namespace hg {
	class IllegalTypeError final : public std::runtime_error {
		using std::runtime_error::runtime_error;
	};
}
