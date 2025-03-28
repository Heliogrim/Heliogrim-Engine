#include "SourceLoader.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Filesystem/Url.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Resource/File.hpp>
#include <Engine.Storage.Registry/IStorageRegistry.hpp>
#include <Engine.Storage/StorageModule.hpp>
#include <Engine.Storage.Registry/Url/FileUrl.hpp>
#include <Engine.Storage.Registry/Url/Url.hpp>

using namespace hg::engine::resource::loader;
using namespace hg;

[[nodiscard]] static string to_string(cref<asset_guid> guid_) {
	return std::format(R"({:08x}-{:04x}-{:04x}-{:016x})", guid_.pre, guid_.c0, guid_.c1, guid_.post);
}

[[nodiscard]] static engine::storage::FileUrl getLfsUrl(const non_owning_rptr<const engine::assets::Asset> asset_);

SourceLoader::SourceLoader(_In_ cref<StorageModule> storageModule_) noexcept :
	SourceLoaderStage(),
	_storage(std::addressof(storageModule_)) {}

SourceLoaderResponse<void>::type SourceLoader::operator()(
	mref<request_type::type> request_,
	mref<request_type::options> options_
) const {

	static_assert(not std::is_void_v<SourceLoaderResponse<void>::type>);

	/**/

	auto lfsUrl = getLfsUrl(request_);
	if (lfsUrl.path().empty()) {
		IM_CORE_WARNF(
			R"(Could not find source data at lfs for asset `{} -> {}`.)",
			encodeGuid4228(request_->get_guid()),
			request_->getAssetName()
		);
		return { *_storage->getSystem(), Arci<storage::IStorage> {} };
	}

	/**/

	auto file = ::hg::fs::File { lfsUrl.path() };
	if (not file.exists() || file.isDirectory()) {
		IM_CORE_ERRORF(
			R"(Lfs source data for asset `{} -> {}` does not exist.)",
			encodeGuid4228(request_->get_guid()),
			request_->getAssetName()
		);
		return { *_storage->getSystem(), Arci<storage::IStorage> {} };
	}

	/**/

	return { *_storage->getSystem(), _storage->getRegistry()->getStorageByUrl(std::move(lfsUrl)) };
}

SourceLoaderStreamResponse<void>::type SourceLoader::operator()(
	mref<stream_request_type::type> request_,
	mref<stream_request_type::options> options_
) const {
	static_assert(not std::is_void_v<SourceLoaderStreamResponse<void>::type>);
	::hg::todo_panic();
}

/* Asset Specific Implementation */

#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets.Type/Geometry/StaticGeometry.hpp>
#include <Engine.Assets.Type/Texture/Font.hpp>
#include <Engine.Assets.Type/Texture/Image.hpp>
#include <Engine.Assets.Type/Texture/TextureAsset.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Reflect/Cast.hpp>

engine::storage::FileUrl getLfsUrl(const non_owning_rptr<const engine::assets::Asset> asset_) {

	switch (asset_->getTypeId().data) {
		case engine::assets::TextureAsset::typeId.data: {

			const auto* const texture = static_cast<const ptr<const engine::assets::TextureAsset>>(asset_);
			const auto baseImageGuid = texture->baseImage();

			const auto registry = engine::Engine::getEngine()->getAssets()->getRegistry();
			const auto asset = registry->findAssetByGuid(baseImageGuid);

			if (asset == None || asset.value() == nullptr) {
				return engine::storage::FileUrl {};
			}

			engine::storage::FileUrl lfsUrl {};
			//const auto* const image = Cast<engine::assets::Image, engine::assets::Asset, false>(asset);
			const auto image = Cast<engine::assets::Image>(asset->get());

			for (const auto& sourceUrl : image->sources()) {
				if (sourceUrl.scheme() == "file") {
					lfsUrl = engine::storage::FileUrl {
						clone(engine::storage::FileScheme),
						clone(sourceUrl.path())
					};
					break;
				}
			}

			return lfsUrl;
		}
		case engine::assets::StaticGeometry::typeId.data: {

			const auto* const geom = static_cast<const ptr<const engine::assets::StaticGeometry>>(asset_);

			engine::storage::FileUrl lfsUrl {};
			for (const auto& sourceUrl : geom->sources()) {
				if (sourceUrl.scheme() == "file") {
					lfsUrl = engine::storage::FileUrl {
						clone(engine::storage::FileScheme),
						clone(sourceUrl.path())
					};
					break;
				}
			}

			return lfsUrl;
		}
		case engine::assets::Font::typeId.data: {

			const auto* const font = static_cast<const ptr<const engine::assets::Font>>(asset_);

			engine::storage::FileUrl lfsUrl {};
			for (const auto& sourceUrl : font->sources()) {
				if (sourceUrl.scheme() == "file") {
					lfsUrl = engine::storage::FileUrl {
						clone(engine::storage::FileScheme),
						clone(sourceUrl.path())
					};
					break;
				}
			}

			return lfsUrl;
		}
		default: {
			return {};
		}
	}

}
