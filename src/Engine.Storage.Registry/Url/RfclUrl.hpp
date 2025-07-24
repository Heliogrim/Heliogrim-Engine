#pragma once

#include <Engine.Common/Forward.hpp>
#include <Engine.Common/Move.hpp>

#include "IUrlComp.hpp"

namespace hg::engine::storage {
	class RfclUrl :
		public IUrlComp<RfclUrl> {
	public:
		using this_type = RfclUrl;

	public:
		constexpr RfclUrl() = default;

		constexpr RfclUrl(auto&& scheme_, mref<String> data_) noexcept :
			IUrlComp(),
			_scheme(::hg::forward<decltype(scheme_)>(scheme_)),
			_schemeSpecificData(::hg::move(data_)) {}

		constexpr RfclUrl(mref<this_type> other_) noexcept = default;

		constexpr RfclUrl(cref<this_type> other_) noexcept = default;

		constexpr ~RfclUrl() = default;

	public:
		constexpr ref<RfclUrl> operator=(cref<RfclUrl> other_) noexcept = default;

		constexpr ref<RfclUrl> operator=(mref<RfclUrl> other_) noexcept = default;

	private:
		UrlScheme _scheme;
		String _schemeSpecificData;

	public:
		[[nodiscard]] constexpr UrlScheme scheme() const noexcept {
			return _scheme;
		}

		[[nodiscard]] String encode() const;
	};
}
