#pragma once
#include <stdexcept>

namespace hg::engine::assets::system {
	class RepositoryFailure final :
		public std::runtime_error {
	public:
		RepositoryFailure() noexcept :
			std::runtime_error("Repository failed to handle asset.") {}
	};
}
