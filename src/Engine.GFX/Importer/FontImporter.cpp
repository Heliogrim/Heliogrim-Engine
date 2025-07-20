#include "FontImporter.hpp"

#include <Engine.Assets/Assets.hpp>
#include <Engine.Async/Await/Promise.hpp>
#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Common/Wrapper.hpp>
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

engine::res::Importer<AtomicRefCountedIntrusive<engine::assets::FontAsset>, void*>::import_result_type FontImporter::import(
	cref<res::FileTypeId> typeId_,
	cref<hg::fs::File> file_
) const {

	/**/

	const auto sourceFileName { static_cast<cref<std::filesystem::path>>(file_.path()).filename().string() };
	const auto sourceExt { static_cast<cref<std::filesystem::path>>(file_.path()).extension().string() };
	const auto sourceName { sourceFileName.substr(0, sourceFileName.size() - sourceExt.size()) };

	/**/

	using namespace ::hg::engine::serialization;
	using namespace ::hg::engine::assets;

	/**/

	auto asset = Arci<FontAsset>::create(
		generate_asset_guid(),
		StringView { sourceName },
		AssetReferenceUrl {},
		AssetUrl { AssetPath { file_.path().parentPath() }, AssetName { sourceName } },
		Vector<fs::Url> { { storage::FileScheme, clone(file_.path()) } }
	);

	/**/

	IM_CORE_LOGF(
		"Created new font asset `{}` -> `{}`",
		asset->getAssetName(),
		encodeGuid4228(asset->getAssetGuid())
	);

	/**/

	return makeImportResult(std::move(asset));
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
