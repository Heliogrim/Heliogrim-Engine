#pragma once

#include "Asset.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import std;
import Engine.Diagnostics;
import Engine.Serialization;
#endif

namespace hg::engine::assets {
	/**
	 * Asset load step: migrates `record_` to the current revision, hydrates `target_` and marks it when keys went unread.
	 * Returns false when the context refused the record (the reason is latched on `ctx_`).
	 */
	template <class AssetType_>
	[[nodiscard]] bool hydrateArchivedAsset(
		ref<const serialization::MigrationRegistry> migrations_,
		ref<const serialization::StructScopedSlot> record_,
		ref<serialization::LoadContext> ctx_,
		ref<AssetType_> target_
	) {
		const auto migrated = migrations_.migrateToCurrent(
			AssetType_::typeId,
			serialization::StructScopedSlot { record_ },
			ctx_
		);
		if (not migrated.has_value()) {
			return false;
		}

		const auto& slot = migrated->slot();
		serialization::access::Structure<AssetType_>::hydrate(slot, target_);
		if (slot.reportUnreadKeys()) {
			target_.markUnknownData();
		}
		return not ctx_.failed();
	}

	/**
	 * Store boundary check: refuses (`save-refused`, one note per marked asset) when any asset carries unknown data.
	 */
	[[nodiscard]] extern Result<void, diag::Diagnostics> checkStoreAllowed(std::span<const nmpt<const Asset>> assets_);
}
