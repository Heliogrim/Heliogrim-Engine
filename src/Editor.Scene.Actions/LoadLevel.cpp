#include "LoadLevel.hpp"

#include <Engine.ACS/Serialization/SerializationModule.hpp>
#include <Engine.Serialization/Diagnostic/Codes.hpp>
#include <Engine.Storage.System/StorageSystem.hpp>

import Engine.Storage;
import Engine.Diagnostics;
import Engine.Logging;

using namespace hg::editor::scene::actions;
using namespace hg;

/**/

static Result<Arci<engine::core::Level>, engine::diag::Diagnostics> reject_level(
	_Inout_ ref<engine::diag::DiagnosticSink> sink_,
	_In_ ref<const engine::diag::DiagnosticCodeInfo> code_,
	_In_ mref<String> message_
);

/**/

LoadLevel::LoadLevel(mref<Arci<engine::assets::LevelAsset>> levelAsset_) :
	DelegateAction(
		[this] {
			if (this->_levelAsset == nullptr)
				return;

			auto sink = engine::diag::CollectingSink {};
			auto loaded = loadLevelFromAsset(*this->_levelAsset, sink);
			engine::diag::logDiagnostics(sink.items());
			_level = loaded.has_value() ? ::hg::move(loaded).value() : nullptr;
		},
		[] { ::hg::todo_panic(); }
	),
	_levelAsset(::hg::move(levelAsset_)),
	_level() {}

LoadLevel::~LoadLevel() = default;

Result<Arci<engine::core::Level>, engine::diag::Diagnostics> editor::scene::actions::loadLevelFromAsset(
	ref<const engine::assets::LevelAsset> levelAsset_,
	ref<engine::diag::DiagnosticSink> sink_
) {

	auto engine = engine::Engine::getEngine();
	auto ssys = engine->getStorage()->getSystem();

	/**/

	const auto& dataUrl = levelAsset_.getLevelData();
	if (not dataUrl.is<engine::storage::ArchiveUrl>()) {
		return reject_level(
			sink_,
			engine::serialization::codes::ser("missing-field"),
			std::format("level `{}` has no level data archive", levelAsset_.getAssetName())
		);
	}

	const auto archiveUrl = dataUrl.as<engine::storage::ArchiveUrl>().encode();
	auto dataArchive = ssys->findArchive(clone(dataUrl.as<engine::storage::ArchiveUrl>()));
	if (dataArchive == None) {
		return reject_level(
			sink_,
			engine::serialization::codes::ser("unresolved-reference"),
			std::format("level `{}`: level data archive `{}` is missing", levelAsset_.getAssetName(), archiveUrl)
		);
	}

	/**/

	// Seed without emitting; the error is only reported when the query never ran.
	bool queried = false;
	Result<Arci<engine::core::Level>, engine::diag::Diagnostics> loaded = Unexpected<engine::diag::Diagnostics> {
		engine::diag::Diagnostics {}
	};
	ssys->query(
		std::move(dataArchive).value(),
		[&](engine::resource::StorageReadonlyArchive& archive_) -> void {
			queried = true;
			loaded = engine::core::loadLevel(archive_, archiveUrl, sink_, Arci<engine::core::Level>::create(levelAsset_));
		}
	);

	if (not queried) {
		return reject_level(
			sink_,
			engine::serialization::codes::ser("truncated"),
			std::format("level `{}`: level data archive `{}` could not be queried", levelAsset_.getAssetName(), archiveUrl)
		);
	}

	return loaded;
}

/**/

static Result<Arci<engine::core::Level>, engine::diag::Diagnostics> reject_level(
	ref<engine::diag::DiagnosticSink> sink_,
	ref<const engine::diag::DiagnosticCodeInfo> code_,
	mref<String> message_
) {
	engine::diag::Diagnostic diagnostic = engine::diag::Diagnostic::error(code_, ::hg::move(message_));
	sink_.emit(engine::diag::Diagnostic { diagnostic });

	engine::diag::Diagnostics out {};
	out.emplace_back(::hg::move(diagnostic));
	return Unexpected<engine::diag::Diagnostics> { ::hg::move(out) };
}
