#pragma once

#include <concepts>
#include <Engine.Asserts/Asserts.hpp>

namespace hg {
	class Asset;
}

namespace hg {
	// Note: Using class instead of concept constraints will make it easier to friend or derive this function struct.
	template <class AssetType_>
	struct FactoryFn {
		[[nodiscard]] constexpr decltype(auto) operator()(auto&&...) const {
			::hg::assertrt(false);
		}
	};

	template <std::derived_from<Asset> AssetType_>
	constexpr inline FactoryFn<AssetType_> CreateAsset;
}
