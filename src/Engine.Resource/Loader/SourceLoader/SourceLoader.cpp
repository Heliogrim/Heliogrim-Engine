#include "SourceLoader.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Logging/Logger.hpp>

#include "../../Source/FileSource.hpp"
#include "Engine.Assets.System/IAssetRegistry.hpp"
#include "Engine.Filesystem/Url.hpp"
#include "Engine.Resource/File.hpp"

using namespace hg::engine::resource::loader;
using namespace hg;

[[nodiscard]] static string to_string(cref<asset_guid> guid_) {
    return std::format(R"({:08x}-{:04x}-{:04x}-{:016x})", guid_.pre, guid_.c0, guid_.c1, guid_.post);
}

[[nodiscard]] static fs::Url getLfsUrl(const non_owning_rptr<const engine::assets::Asset> asset_);

SourceLoader::SourceLoader() = default;

SourceLoader::~SourceLoader() noexcept = default;

SourceLoaderResponse<void>::type SourceLoader::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_
) const {

    static_assert(std::is_same_v<SourceLoaderResponse<void>::type, smr<Source>>);

    /**/

    const auto lfsUrl = getLfsUrl(request_);
    if (lfsUrl.empty()) {
        IM_CORE_WARNF(
            R"(Could not find source data at lfs for asset `{} -> {}`.)",
            std::to_string(request_->get_guid()),
            request_->getAssetName()
        );
        return smr<Source> {};
    }

    /**/

    hg::fs::File file { lfsUrl.path().string() };
    if (not file.exists() || file.isDirectory()) {
        IM_CORE_ERRORF(
            R"(Lfs source data for asset `{} -> {}` does not exist.)",
            std::to_string(request_->get_guid()),
            request_->getAssetName()
        );
        return smr<Source> {};
    }

    /**/

    return make_smr<FileSource>(std::move(file));
}

SourceLoaderStreamResponse<void>::type SourceLoader::operator()(
    mref<stream_request_type::type> request_,
    mref<stream_request_type::options> options_
) const {

    static_assert(std::is_same_v<SourceLoaderStreamResponse<void>::type, smr<Source>>);

    return {};
}

/* Asset Specific Implementation */

#include <Engine.Assets/Types/Image.hpp>
#include <Engine.Assets/Types/Texture/TextureAsset.hpp>
#include <Engine.Assets/Types/Geometry/StaticGeometry.hpp>
#include <Engine.Assets/Types/Font.hpp>

#include <Engine.Core/Engine.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Reflect/Cast.hpp>

fs::Url getLfsUrl(const non_owning_rptr<const engine::assets::Asset> asset_) {

    switch (asset_->getTypeId().data) {
        case engine::assets::TextureAsset::typeId.data: {

            const auto* const texture = static_cast<const ptr<const engine::assets::TextureAsset>>(asset_);
            const auto baseImageGuid = texture->baseImage();

            const auto* const registry = engine::Engine::getEngine()->getAssets()->getRegistry();
            const auto* const asset = registry->findAssetByGuid(baseImageGuid);

            if (asset == nullptr) {
                return fs::Url {};
            }

            fs::Url lfsUrl {};
            //const auto* const image = Cast<engine::assets::Image, engine::assets::Asset, false>(asset);
            const auto* const image = static_cast<const ptr<const engine::assets::Image>>(asset);

            for (const auto& sourceUrl : image->sources()) {
                if (sourceUrl.scheme() == "file") {
                    lfsUrl = sourceUrl;
                    break;
                }
            }

            return lfsUrl;
        }
        case engine::assets::StaticGeometry::typeId.data: {

            const auto* const geom = static_cast<const ptr<const engine::assets::StaticGeometry>>(asset_);

            fs::Url lfsUrl {};
            for (const auto& sourceUrl : geom->sources()) {
                if (sourceUrl.scheme() == "file") {
                    lfsUrl = sourceUrl;
                    break;
                }
            }

            return lfsUrl;
        }
        case engine::assets::Font::typeId.data: {

            const auto* const font = static_cast<const ptr<const engine::assets::Font>>(asset_);

            fs::Url lfsUrl {};
            for (const auto& sourceUrl : font->sources()) {
                if (sourceUrl.scheme() == "file") {
                    lfsUrl = sourceUrl;
                    break;
                }
            }

            return lfsUrl;
        }
        default: {
            return fs::Url {};
        }
    }

}
