#include "SourceLoader.hpp"

#include <Engine.Common/Url.hpp>
#include <Engine.Logging/Logger.hpp>

#include "../../Source/FileSource.hpp"

using namespace ember::engine::resource::loader;
using namespace ember;

[[nodiscard]] static string to_string(cref<asset_guid> guid_) {
    return _STD format(R"({:08x}-{:04x}-{:04x}-{:016x})", guid_.pre, guid_.c0, guid_.c1, guid_.post);
}

[[nodiscard]] static Url getLfsUrl(const non_owning_rptr<engine::assets::Asset> asset_);

constexpr SourceLoader::SourceLoader() = default;

SourceLoader::~SourceLoader() noexcept = default;

SourceLoaderResponse<void>::type SourceLoader::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_
) const {

    static_assert(_STD is_same_v<SourceLoaderResponse<void>::type, smr<Source>>);

    /**/

    const auto lfsUrl = getLfsUrl(request_);
    if (lfsUrl.empty()) {
        IM_CORE_WARNF(
            R"(Could not find source data at lfs for asset `{} -> {}`.)",
            _STD to_string(request_->get_guid()),
            request_->getAssetName()
        );
        return smr<Source> {};
    }

    /**/

    fs::File file { lfsUrl.path() };
    if (not file.exists() || file.isDirectory()) {
        IM_CORE_ERRORF(
            R"(Lfs source data for asset `{} -> {}` does not exist.)",
            _STD to_string(request_->get_guid()),
            request_->getAssetName()
        );
        return smr<Source> {};
    }

    /**/

    return make_smr<FileSource>(_STD move(file));
}

SourceLoaderStreamResponse<void>::type SourceLoader::operator()(
    mref<stream_request_type::type> request_,
    mref<stream_request_type::options> options_
) const {

    static_assert(_STD is_same_v<SourceLoaderStreamResponse<void>::type, smr<Source>>);

    return {};
}

/* Asset Specific Implementation */

#include <Engine.Assets/Types/Image.hpp>
#include <Engine.Assets/Types/Texture/Texture.hpp>
#include <Engine.Assets/Types/Geometry/StaticGeometry.hpp>
#include <Engine.Assets/Types/Font.hpp>

#include <Engine.Core/Engine.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets/Database/AssetDatabase.hpp>
#include <Engine.Reflect/Cast.hpp>

Url getLfsUrl(const non_owning_rptr<engine::assets::Asset> asset_) {

    switch (asset_->getTypeId().data) {
        case engine::assets::Texture::typeId.data: {

            const auto* const texture = static_cast<const ptr<engine::assets::Texture>>(asset_);
            const auto baseImageGuid = texture->baseImage();

            const auto* const database = engine::Engine::getEngine()->getAssets()->getDatabase();
            const auto query = database->query(baseImageGuid);

            if (not query.exists()) {
                return Url {};
            }

            // TODO: const auto* const image = Cast<engine::assets::Image>(query.get(_STD nothrow));
            const auto* const image = static_cast<const ptr<const engine::assets::Image>>(query.get(traits::nothrow));

            Url lfsUrl {};
            for (const auto& sourceUrl : image->sources()) {
                if (sourceUrl.scheme() == "file") {
                    lfsUrl = sourceUrl;
                    break;
                }
            }

            return lfsUrl;
        }
        case engine::assets::StaticGeometry::typeId.data: {

            const auto* const geom = static_cast<const ptr<engine::assets::StaticGeometry>>(asset_);

            Url lfsUrl {};
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

            Url lfsUrl {};
            for (const auto& sourceUrl : font->sources()) {
                if (sourceUrl.scheme() == "file") {
                    lfsUrl = sourceUrl;
                    break;
                }
            }

            return lfsUrl;
        }
        default: {
            return Url {};
        }
    }

}
