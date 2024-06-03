#pragma once

#include "IUrlComp.hpp"

namespace hg::engine::storage {
	class RfclUrl :
		public IUrlComp<RfclUrl> {
	public:
		using this_type = RfclUrl;

	public:
		constexpr RfclUrl() = default;

		constexpr ~RfclUrl() = default;

	private:
		UrlScheme _scheme;
		fs::Path _path;

	public:
		[[nodiscard]] constexpr UrlScheme scheme() const noexcept {
			return _scheme;
		}
	};
}
