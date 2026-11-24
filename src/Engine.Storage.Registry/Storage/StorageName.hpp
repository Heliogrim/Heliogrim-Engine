#pragma once

#include <Engine.Common/Sal.hpp>

#include "LocalFileStorage.hpp"
#include "../IStorage.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Reflect;
#endif

namespace hg::engine::storage {
	/**
	 * Names the storage for diagnostics; the file path for local files, the backing's name for secondary storages.
	 */
	[[nodiscard]] inline String storageName(_In_ ref<const IStorage> storage_) {
		if (IsType<system::LocalFileStorage>(storage_)) {
			return static_cast<String>(static_cast<ref<const system::LocalFileStorage>>(storage_)._lfsPath);
		}

		if (not storage_.isPrimaryStorage()) {
			const auto backing = static_cast<ref<const ISecondaryStorage>>(storage_).getBacking();
			return backing != nullptr ? storageName(*backing) : String { "<unbacked storage>" };
		}

		return String { "<in-memory storage>" };
	}
}
