#pragma once

#include "IUrlComp.hpp"

namespace hg::engine::storage {
	class RfclUrl :
		public IUrlComp<RfclUrl> {
	public:
		using this_type = RfclUrl;

	public:
		constexpr RfclUrl() = default;

		constexpr RfclUrl(mref<this_type> other_) noexcept = default;

		constexpr RfclUrl(cref<this_type> other_) noexcept = default;

		constexpr ~RfclUrl() = default;

	public:
		constexpr ref<RfclUrl> operator=(cref<RfclUrl> other_) noexcept = default;

		constexpr ref<RfclUrl> operator=(mref<RfclUrl> other_) noexcept = default;

	private:
		UrlScheme _scheme;
		fs::Path _path;

	public:
		[[nodiscard]] constexpr UrlScheme scheme() const noexcept {
			return _scheme;
		}

		[[nodiscard]] String encode() const;
	};
}
