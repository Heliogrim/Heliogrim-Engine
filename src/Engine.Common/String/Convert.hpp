#pragma once

#include "../String.hpp"
#include "../Wrapper.hpp"

namespace hg {
	[[nodiscard]] extern std::wstring stringToWString(ref<const std::string> string_);
}
