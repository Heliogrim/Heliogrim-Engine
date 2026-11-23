#pragma once

#include <Engine.Common/Optional.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Serialization/SerializationModule.hpp>
#include <Engine.Serialization/Schema/Migration.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

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
		if (host == nullptr || host->getSerialization() == nullptr) {
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

		return host->getSerialization()->migrations().migrateToCurrent(
			type_,
			record_,
			*ctx
		);
	}
}
