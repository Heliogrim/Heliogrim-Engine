#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import std;
#endif

namespace hg::engine::storage {
	class AccessError final : public std::runtime_error {
		using std::runtime_error::runtime_error;
	};
}
