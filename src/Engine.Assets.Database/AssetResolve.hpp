#pragma once

#include "AssetDatabase.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Serialization;
#endif

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
