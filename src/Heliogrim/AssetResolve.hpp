#pragma once

#include <concepts>
#include <utility>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Serialization/Load/ReadError.hpp>

#include "AssetDatabase.hpp"

namespace hg {
	/**
	 * Null-safe resolve of a file-derived asset guid; a missing asset module or asset is an unresolved reference warning.
	 */
	template <std::derived_from<AssetHandle> HandleType_, typename Slot_>
	[[nodiscard]] Opt<HandleType_> ResolveAssetHandle(ref<const Slot_> slot_, ref<const AssetGuid> guid_) {
		if (not IsAssetsAvailable()) {
			slot_.warn(engine::serialization::ReadError::Kind::eUnresolvedReference, "asset module unavailable");
			return None;
		}

		auto found = GetAssets().find<HandleType_>(guid_);
		if (not found) {
			slot_.warn(engine::serialization::ReadError::Kind::eUnresolvedReference, "asset not found");
			return None;
		}

		return Some(std::move(found.value));
	}
}
