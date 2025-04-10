#include "AutoImportAction.hpp"

#include <algorithm>
#include <ranges>
#include <Engine.Asserts/Todo.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Sal.hpp>

using namespace hg::editor;
using namespace hg;

/**/

// @formatter:off
[[nodiscard]] static Opt<Vector<Arci<engine::assets::Asset>>> autoImportBundle(cref<engine::storage::FileUrl> bundleUrl_);
[[nodiscard]] static Opt<Vector<Arci<engine::assets::Asset>>> autoImportFile(cref<engine::storage::FileUrl> fileUrl_);
// @formatter:on

/**/

AutoImportAction::AutoImportAction() noexcept :
	InheritMeta() {}

AutoImportAction::AutoImportAction(mref<engine::storage::FileUrl> importFile_) noexcept :
	InheritMeta(),
	_importFile(std::move(importFile_)),
	_generatedAssets() {}

AutoImportAction::~AutoImportAction() = default;

engine::storage::FileUrl AutoImportAction::getImportFile() const noexcept {
	return _importFile;
}

cref<decltype(AutoImportAction::_generatedAssets)> AutoImportAction::generatedAssets() const noexcept {
	return _generatedAssets;
}

void AutoImportAction::setRunning() {
	_finished.clear();
	_running.test_and_set();
}

void AutoImportAction::setFinished() {
	_running.clear();
	_finished.test_and_set();
}

bool AutoImportAction::isReversible() const noexcept {
	return false;
}

bool AutoImportAction::isPending() const noexcept {
	return not _running.test() && not _finished.test();
}

bool AutoImportAction::isRunning() const noexcept {
	return _running.test();
}

bool AutoImportAction::isFinished() const noexcept {
	return _finished.test();
}

Result<void, std::runtime_error> AutoImportAction::apply() {

	setRunning();
	_failed = not std::filesystem::exists(_importFile.path());

	/**/

	if (not _failed) {

		if (std::filesystem::is_directory(_importFile.path())) {
			auto assets = autoImportBundle(_importFile);
			if (not(_failed = not assets.has_value())) {
				_generatedAssets = std::move(assets.take().value());
			}
		} else {
			auto assets = autoImportFile(_importFile);
			if (not(_failed = not assets.has_value())) {
				_generatedAssets = std::move(assets.take().value());
			}
		}

	}

	/**/

	setFinished();
	return Expected<void> {};
}

Result<void, std::runtime_error> AutoImportAction::revoke() {
	return Unexpected { std::runtime_error { "Unable to revoke auto import action." } };
}

Result<void, std::runtime_error> AutoImportAction::undo() {
	::hg::todo_panic();
}

AutoImportAction::operator ptr<await_signal_sub_type>() const noexcept {
	return &_finished;
}

/**/

#include <Engine.Assets.Type/Geometry/StaticGeometry.hpp>
#include <Engine.Assets.Type/Texture/Font.hpp>
#include <Engine.Assets.Type/Texture/Image.hpp>
#include <Engine.Assets.Type/Texture/TextureAsset.hpp>
#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Importer/FontFileTypes.hpp>
#include <Engine.GFX/Importer/ImageFileTypes.hpp>
#include <Engine.GFX/Importer/ModelFileTypes.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Reflect/TypeSwitch.hpp>
#include <Engine.Resource.Archive/BufferArchive.hpp>
#include <Engine.Resource.Archive/StorageReadWriteArchive.hpp>
#include <Engine.Resource/FileTypeId.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Archive/StructuredArchive.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Storage.Registry/IStorageRegistry.hpp>
#include <Engine.Storage/StorageModule.hpp>
#include <Engine.Storage.Registry/Options/FileStorageDescriptor.hpp>
#include <Engine.Storage.Registry/Options/PackageStorageDescriptor.hpp>
#include <Engine.Storage.Registry/Options/StorageDescriptor.hpp>
#include <Engine.Storage.Registry/Storage/PackageStorage.hpp>
#include <Engine.Storage.Registry/Url/Url.hpp>
#include <Engine.Storage.System/StorageSystem.hpp>

/**/

using namespace hg::engine::resource::package;
using namespace hg::engine::resource;

// @formatter:off
[[nodiscard]] static Opt<Vector<Arci<engine::assets::Asset>>> autoImportCombinedImageTexture(_In_ mref<engine::res::FileTypeId> fileTypeId_, _In_ cref<engine::storage::FileUrl> fileUrl_);
[[nodiscard]] static Opt<Vector<Arci<engine::assets::Asset>>> autoImportFont(_In_ mref<engine::res::FileTypeId> fileTypeId_, _In_ cref<engine::storage::FileUrl> fileUrl_);
[[nodiscard]] static Opt<Vector<Arci<engine::assets::Asset>>> autoImportStaticGeometry(_In_ mref<engine::res::FileTypeId> fileTypeId_, _In_ cref<engine::storage::FileUrl> fileUrl_);

[[nodiscard]] static Opt<Arci<engine::storage::system::PackageStorage>> autoStoreAssets(_In_ cref<engine::storage::FileUrl> baseFileUrl_, _In_ cref<Vector<Arci<engine::assets::Asset>>> assets_);

[[nodiscard]] static BufferArchive autoStoreFont(_In_ nmpt<engine::assets::Font> fontAsset_);
[[nodiscard]] static BufferArchive autoStoreImage(_In_ nmpt<engine::assets::Image> imageAsset_);
[[nodiscard]] static BufferArchive autoStoreTexture(_In_ nmpt<engine::assets::TextureAsset> textureAsset_);
[[nodiscard]] static BufferArchive autoStoreStaticGeometry(_In_ nmpt<engine::assets::StaticGeometry> staticGeometryAsset_);
// @formatter:on

/**/

struct BundleFileType {
	std::filesystem::path filePath;
	engine::res::FileTypeId fileTypeId;
	u8 precedence;
};

/**/

[[nodiscard]] static Opt<engine::res::FileTypeId> queryFileTypeId(_In_ cref<std::filesystem::path> path_) {
	const auto extension = path_.extension().string();
	auto registered = engine::res::FileTypeRegister::make().getByExt(extension);
	return registered.valid() ? Some(std::move(registered)) : None;
}

[[nodiscard]] static u8 queryFileTypePrecedence(cref<engine::res::FileTypeId> fileTypeId_) {
	// TODO: Introduce precedence or explicit ordering while registering to provide some kind of priority.
	return fileTypeId_.ext.size();
}

[[nodiscard]] static Vector<BundleFileType> queryBundleTypeIdByPrecedence(
	_In_ cref<std::filesystem::path> path_
) {

	auto range = std::filesystem::directory_iterator {
		path_, std::filesystem::directory_options::follow_directory_symlink
	};

	Vector<BundleFileType> types {};
	for (const auto& directoryEntry : range) {

		auto fileTypeId = queryFileTypeId(directoryEntry.path());
		if (not fileTypeId) {
			continue;
		}

		auto precedence = queryFileTypePrecedence(fileTypeId.value());
		types.emplace_back(directoryEntry.path(), std::move(fileTypeId.take().value()), precedence);
	}

	/**/

	std::ranges::sort(
		types,
		std::greater<u8> {},
		[](const auto& entry_) {
			return entry_.precedence;
		}
	);

	return types;
}

static void associateBundle(
	_Inout_ ref<Vector<Arci<engine::assets::Asset>>> assets_,
	_In_ mref<Vector<BundleFileType>> bundle_
) {
	// TODO:
}

Opt<Vector<Arci<engine::assets::Asset>>> autoImportBundle(cref<engine::storage::FileUrl> bundleUrl_) {

	auto fileTypes = queryBundleTypeIdByPrecedence(bundleUrl_.path());
	if (fileTypes.empty()) {
		return None;
	}

	/**/

	Opt<Vector<Arci<engine::assets::Asset>>> result = None;

	auto fileTypeIt = fileTypes.begin();
	while (fileTypeIt != fileTypes.end()) {

		if (std::ranges::contains(engine::gfx::ImageFileType::asSpan(), fileTypeIt->fileTypeId)) {
			result = autoImportCombinedImageTexture(
				clone(fileTypeIt->fileTypeId),
				engine::storage::FileUrl { bundleUrl_.scheme(), engine::fs::Path { fileTypeIt->filePath } }
			);
			break;
		}

		if (std::ranges::contains(engine::gfx::ModelFileType::asSpan(), fileTypeIt->fileTypeId)) {
			result = autoImportStaticGeometry(
				clone(fileTypeIt->fileTypeId),
				engine::storage::FileUrl { bundleUrl_.scheme(), engine::fs::Path { fileTypeIt->filePath } }
			);
			break;
		}

	}

	/**/

	if (result.has_value()) {
		associateBundle(result.value(), std::move(fileTypes));
		// Question: Do we need to capture the result within the action report to handle the failed state?
		std::ignore = autoStoreAssets(bundleUrl_, result.value());
	}

	/**/

	return result;
}

Opt<Vector<Arci<engine::assets::Asset>>> autoImportFile(cref<engine::storage::FileUrl> fileUrl_) {

	auto fileTypeId = queryFileTypeId(fileUrl_.path());
	if (not fileTypeId.has_value()) {
		return None;
	}

	/**/

	Opt<Vector<Arci<engine::assets::Asset>>> result = None;

	/**/

	if (std::ranges::contains(engine::gfx::ImageFileType::asSpan(), fileTypeId.value())) {
		result = autoImportCombinedImageTexture(std::move(fileTypeId.take().value()), fileUrl_);

	} else if (std::ranges::contains(engine::gfx::ModelFileType::asSpan(), fileTypeId.value())) {
		result = autoImportStaticGeometry(std::move(fileTypeId.take().value()), fileUrl_);

	} else if (std::ranges::contains(engine::gfx::FontFileType::asSpan(), fileTypeId.value())) {
		result = autoImportFont(std::move(fileTypeId.take().value()), fileUrl_);
	}

	/**/

	if (result.has_value()) {
		// Question: Do we need to capture the result within the action report to handle the failed state?
		std::ignore = autoStoreAssets(fileUrl_, result.value());
	}

	/**/

	return result;
}

/**/

Opt<Vector<Arci<engine::assets::Asset>>> autoImportCombinedImageTexture(
	mref<engine::res::FileTypeId> fileTypeId_,
	cref<engine::storage::FileUrl> fileUrl_
) {

	const auto& importer = engine::Engine::getEngine()->getResources()->importer();
	if (not importer.hasImporter(fileTypeId_)) {
		return None;
	}

	/**/

	const auto result = importer.import<std::pair<Arci<engine::assets::TextureAsset>, Arci<engine::assets::Image>>>(
		fileTypeId_,
		{ fileUrl_.path() }
	).get();

	if (result.first == nullptr || result.second == nullptr) {
		::hg::assertrt(result.first == nullptr && result.second == nullptr);
		return None;
	}

	/**/

	return Some(
		std::vector {
			std::move(result.first).into<engine::assets::Asset>(),
			std::move(result.second).into<engine::assets::Asset>()
		}
	);
}

Opt<Vector<Arci<engine::assets::Asset>>> autoImportFont(
	mref<engine::res::FileTypeId> fileTypeId_,
	cref<engine::storage::FileUrl> fileUrl_
) {

	const auto& importer = engine::Engine::getEngine()->getResources()->importer();
	if (not importer.hasImporter(fileTypeId_)) {
		return None;
	}

	/**/

	const auto result = importer.import<Arci<engine::assets::Font>>(
		fileTypeId_,
		{ fileUrl_.path() }
	).get();

	if (result != nullptr) {
		return Some(std::vector { std::move(result).into<engine::assets::Asset>() });
	}

	return None;
}

Opt<Vector<Arci<engine::assets::Asset>>> autoImportStaticGeometry(
	mref<engine::res::FileTypeId> fileTypeId_,
	cref<engine::storage::FileUrl> fileUrl_
) {

	const auto& importer = engine::Engine::getEngine()->getResources()->importer();
	if (not importer.hasImporter(fileTypeId_)) {
		return None;
	}

	/**/

	const auto result = importer.import<Arci<engine::assets::StaticGeometry>>(
		fileTypeId_,
		{ fileUrl_.path() }
	).get();

	if (result != nullptr) {
		return Some(std::vector { std::move(result).into<engine::assets::Asset>() });
	}

	return None;
}

#if ENV_WIN
constexpr static auto prefix_extension = L'.';
constexpr static auto packed_extension = std::wstring_view { L".imp" };
#else
constexpr static auto prefix_extension = '.';
constexpr static auto packed_extension = std::string_view { R"(.imp)" };
#endif

Opt<Arci<engine::storage::system::PackageStorage>> autoStoreAssets(
	cref<engine::storage::FileUrl> baseFileUrl_,
	cref<Vector<Arci<engine::assets::Asset>>> assets_
) {

	const auto packageFileUrl = std::filesystem::path { baseFileUrl_.path() }.concat(packed_extension);
	if (std::filesystem::exists(packageFileUrl)) {
		IM_CORE_ERRORF(
			"Failed to auto generate the package file `{}`, cause the targeted package file does already exist.",
			packageFileUrl.string()
		);
		return None;
	}

	/**/

	const auto engine = engine::Engine::getEngine();
	const auto storeReg = engine->getStorage()->getRegistry();
	const auto storeSys = engine->getStorage()->getSystem();

	/* Generate the package storage unit */

	const auto storageFileUrl = engine::storage::FileUrl {
		clone(engine::storage::FileProjectScheme), fs::Path { packageFileUrl }
	};
	auto storageUnit = [](decltype(storeReg) registry_, const auto& fileUrl_) {

		if (registry_->hasStorage(clone(fileUrl_))) {
			return registry_->getStorageByUrl(clone(fileUrl_));
		}

		return registry_->insert(engine::storage::FileStorageDescriptor { clone(fileUrl_) });

	}(storeReg, storageFileUrl);

	if (storageUnit == nullptr) {
		IM_CORE_ERRORF(
			"Failed to auto generate the package storage unit with backing file `{}`.",
			packageFileUrl.string()
		);
		return None;
	}

	/* Morph the underlying storage unit into a package storage */

	auto maybePackage = storeSys->requestPackage(
		{ .storage = ::hg::move(storageUnit).into<engine::storage::system::LocalFileStorage>() }
	);

	if (not maybePackage.has_value()) {
		IM_CORE_ERRORF(
			"Failed to request newly generate package based on file url storage `{}`.",
			packageFileUrl.string()
		);
		return None;
	}

	auto packageStore = ::hg::move(maybePackage).value();

	/**/

	/* Collect serialized data to write */

	auto serialized = Vector<BufferArchive> {};
	serialized.reserve(assets_.size());

	for (const auto& asset : assets_) {
		serialized.emplace_back(
			switchType(
				asset.get(),
				[](const ptr<engine::assets::Image> imageAsset_) {
					return autoStoreImage(imageAsset_);
				},
				[](const ptr<engine::assets::TextureAsset> textureAsset_) {
					return autoStoreTexture(textureAsset_);
				},
				[](const ptr<engine::assets::StaticGeometry> staticGeometryAsset_) {
					return autoStoreStaticGeometry(staticGeometryAsset_);
				},
				[](const ptr<engine::assets::Font> fontAsset_) {
					return autoStoreFont(fontAsset_);
				}
			)
		);
	}

	/* Write data collection to package */

	auto ioFailed = false;
	for (auto idx = 0uLL; idx < serialized.size(); ++idx) {

		auto result = storeSys->addArchiveToPackage(
			clone(packageStore),
			serialized[idx]
		);

		if (not result.has_value()) {
			IM_CORE_ERRORF(
				"Failed to store serialized asset data of `{}` into auto generated package `{}`.",
				encodeGuid4228(assets_[idx]->get_guid()),
				packageFileUrl.string()
			);
			ioFailed = true;
			break;
		}
	}

	/* Recover from storing failure */

	if (ioFailed) {
		// Note: We need to improve the error recovery... This is just a dirty quickfix.
		std::ignore = storeSys->removeStorage(clone(storageFileUrl));
		std::filesystem::remove(packageFileUrl);
		return None;
	}

	/**/

	return Some(std::move(packageStore));
}

/**/
#pragma region Asset Specific Serialization

BufferArchive autoStoreFont(
	nmpt<engine::assets::Font> fontAsset_
) {

	auto archive = BufferArchive {};

	auto structured = engine::serialization::StructuredArchive { archive };
	engine::serialization::access::Structure<engine::assets::Font>::serialize(
		*fontAsset_,
		structured.insertRootSlot().intoStruct()
	);

	archive.setType(ArchiveType::eSerializedStructure);
	return archive;
}

BufferArchive autoStoreImage(
	nmpt<engine::assets::Image> imageAsset_
) {

	auto archive = BufferArchive {};

	auto structured = engine::serialization::StructuredArchive { archive };
	engine::serialization::access::Structure<engine::assets::Image>::serialize(
		*imageAsset_,
		structured.insertRootSlot().intoStruct()
	);

	archive.setType(ArchiveType::eSerializedStructure);
	return archive;
}

BufferArchive autoStoreTexture(
	nmpt<engine::assets::TextureAsset> textureAsset_
) {

	auto archive = BufferArchive {};

	auto structured = engine::serialization::StructuredArchive { archive };
	engine::serialization::access::Structure<engine::assets::TextureAsset>::serialize(
		*textureAsset_,
		structured.insertRootSlot().intoStruct()
	);

	archive.setType(ArchiveType::eSerializedStructure);
	return archive;
}

BufferArchive autoStoreStaticGeometry(
	nmpt<engine::assets::StaticGeometry> staticGeometryAsset_
) {

	auto archive = BufferArchive {};

	auto structured = engine::serialization::StructuredArchive { archive };
	engine::serialization::access::Structure<engine::assets::StaticGeometry>::serialize(
		*staticGeometryAsset_,
		structured.insertRootSlot().intoStruct()
	);

	archive.setType(ArchiveType::eSerializedStructure);
	return archive;
}

#pragma endregion
