#pragma once

#include <Engine.ACS/Serialization/SerializationModule.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Core;
import Engine.Serialization;
#endif

namespace hg {
	/* Upgrades `record_` to the current revision of `type_`; nullopt when refused, the reason is already in the load context. */
	[[nodiscard]] inline Opt<engine::serialization::Migrated> migrateArchetypeRecord(
		const type_id type_,
		ref<const engine::serialization::StructScopedSlot> record_
	) {
		const auto& root = record_.slot()->getState().root;
		auto ctx = root != nullptr ? root->context : nullptr;
		// shortcut: a contextless load hydrates as written, without migration; give every load a context to upgrade
		if (ctx == nullptr) {
			return SomeVal<engine::serialization::Migrated>(engine::serialization::StructScopedSlot { record_ });
		}

		const auto host = engine::Engine::getEngine();
		if (host == nullptr || not engine::SerializationModule::get().has_value()) {
			ctx->fail(
				engine::serialization::ReadError {
					engine::serialization::ReadError::Kind::eMigrationFailed,
					record_.slot()->getState().offset,
					0LL,
					"no serialization module to migrate the record"
				}
			);
			return None;
		}

		return engine::SerializationModule::get()->migrations().migrateToCurrent(
			type_,
			record_,
			*ctx
		);
	}
}
