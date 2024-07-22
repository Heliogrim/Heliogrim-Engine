#include "FontImporter.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Logging/Logger.hpp>

using namespace hg::engine::gfx;
using namespace hg;

/**/

[[nodiscard]] static FontImporter::import_result_type makeImportResult(mref<FontImporter::import_type> value_);

/**/

FontImporter::FontImporter() = default;

FontImporter::~FontImporter() = default;

bool FontImporter::canImport(cref<res::FileTypeId> typeId_, cref<hg::fs::File> file_) const noexcept {

	if (typeId_.ext != ".ttf" && typeId_.ext != ".otf") {
		return false;
	}

	return file_.exists();
}

FontImporter::descriptor_type FontImporter::descriptor() const noexcept {
	return nullptr;
}

engine::res::Importer<AtomicRefCountedIntrusive<engine::assets::Font>, void*>::import_result_type FontImporter::import(
	cref<res::FileTypeId> typeId_,
	cref<hg::fs::File> file_
) const {

	/**/

	const auto sourceFileName { file_.path().filename().string() };
	const auto sourceExt { file_.path().extension().string() };
	const auto sourceName { sourceFileName.substr(0, sourceFileName.size() - sourceExt.size()) };

	/**/

	using namespace ::hg::engine::serialization;
	using namespace ::hg::engine::assets;

	/**/

	auto& factory = *Engine::getEngine()->getAssets()->getFactory();
	auto font = factory.createFontAsset(generate_asset_guid(), file_.path().string());
	font->setAssetName(sourceName);

	/**/

	IM_CORE_LOGF(
		"Created new font asset `{}` -> `{}`",
		font->getAssetName(),
		encodeGuid4228(font->get_guid())
	);

	/**/

	return makeImportResult(std::move(font));
}

/**/

FontImporter::import_result_type makeImportResult(mref<FontImporter::import_type> value_) {

	concurrent::promise<FontImporter::import_type> prom {
		[value = std::move(value_)]() {
			return value;
		}
	};

	auto result { prom.get() };
	prom();

	return result;
}
