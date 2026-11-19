#pragma once
#if not defined(HG_MODULE_INTERFACE_UNIT)
import std;
#endif

namespace hg::engine::assets::system {
	class IndexFailure final :
		public std::runtime_error {
	public:
		IndexFailure() noexcept :
			std::runtime_error("Index failed to handle asset.") {}
	};
}
