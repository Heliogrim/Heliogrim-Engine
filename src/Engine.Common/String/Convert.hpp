#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg {
	[[nodiscard]] extern std::wstring stringToWString(ref<const std::string> string_);
}
