#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Managed/Rc.hpp>

#include "../IStorage.hpp"
#include "../Url/Url.hpp"

namespace hg::engine::storage::system {
	template <typename UrlType_>
	[[nodiscard]] constexpr decltype(auto) inlineHasStorage(const auto& storages_, cref<Url> url_) {
		return url_.is<UrlType_>() && storages_.contains(url_.as<UrlType_>().guid());
	}

	template <typename UrlType_>
	[[nodiscard]] constexpr decltype(auto) inlineGetStorageByUrl(const auto& storages_, cref<Url> url_) {
		if (not url_.is<UrlType_>()) {
			return Arci<IStorage> {};
		}

		const auto it = storages_.find(url_.as<UrlType_>().guid());
		return it != storages_.end() ? it->second.template into<IStorage>() : Arci<IStorage> {};
	}

	[[nodiscard]] constexpr decltype(auto) inlineRemoveStorage(auto& storages_, mref<nmpt<IStorage>> storage_) {
		const auto vecIt = std::ranges::find(
			storages_.values(),
			storage_.get(),
			[](const auto& pair_) {
				return pair_.second.get();
			}
		);
		return vecIt != storages_.values().end() && storages_.erase(vecIt->first);
	}
}
