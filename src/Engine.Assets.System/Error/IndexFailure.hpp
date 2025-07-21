#pragma once
#include <stdexcept>

namespace hg::engine::assets::system {
	class IndexFailure final :
		public std::runtime_error {
	public:
		IndexFailure() noexcept :
			std::runtime_error("Index failed to handle asset.") {}
	};
}
