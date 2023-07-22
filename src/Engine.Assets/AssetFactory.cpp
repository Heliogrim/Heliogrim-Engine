#include "AssetFactory.hpp"

#include <filesystem>
#include <Engine.Serialization/Layout/DataLayout.hpp>
#include <Engine.Serialization.Layouts/LayoutManager.hpp>
#include <Engine.Assets.System/AssetDescriptor.hpp>
#include <Engine.Assets.System/AssetRegistry.hpp>
#include <Engine.Assets.System/Repository/InMemAssetRepository.hpp>

#include "Types/Font.hpp"
#include "Types/Material/GfxMaterial.hpp"
#include "Types/Material/GfxMaterialPrototype.hpp"
#include "Types/Image.hpp"
#include "Types/LandscapeGeometry.hpp"
#include "Types/Level.hpp"
#include "Types/PfxMaterial.hpp"
#include "Types/SfxMaterial.hpp"
#include "Types/SkeletalGeometry.hpp"
#include "Types/Sound.hpp"
#include "Types/Geometry/StaticGeometry.hpp"
#include "Types/Texture/Texture.hpp"

#include <sstream>

using namespace hg::engine::assets;
using namespace hg;

/**/

void engine::assets::storeDefaultNameAndUrl(non_owning_rptr<Asset> asset_, string source_) {

    #ifdef _DEBUG
    if (not source_.empty() && asset_->getAssetName().empty()) {
        asset_->setAssetName(_STD filesystem::path(source_).filename().string());
    }
    #endif

    if (asset_->getVirtualUrl().empty()) {

        _STD filesystem::path pseudo {};

        switch (asset_->getTypeId().data) {
            case assets::Image::typeId.data: {
                pseudo = "texture";
                break;
            }
            case assets::Texture::typeId.data: {
                pseudo = "texture";
                break;
            }
            case assets::GfxMaterial::typeId.data: {
                pseudo = "material";
                break;
            }
            case assets::StaticGeometry::typeId.data: {
                pseudo = "geometry";
                break;
            }
            case assets::Font::typeId.data: {
                pseudo = "font";
                break;
            }
            default: {
                pseudo = "content";
                break;
            }
        }

        if (not asset_->getAssetName().empty()) {

            _STD stringstream ss {};
            ss << asset_->getAssetName();
            ss << " :: ";
            ss << asset_->get_guid().pre;
            ss << "-";
            ss << asset_->get_guid().c0;
            ss << "-";
            ss << asset_->get_guid().c1;
            ss << "-";
            ss << asset_->get_guid().post;

            pseudo /= ss.str();

        } else {

            _STD stringstream ss {};
            ss << asset_->get_guid().pre;
            ss << "-";
            ss << asset_->get_guid().c0;
            ss << "-";
            ss << asset_->get_guid().c1;
            ss << "-";
            ss << asset_->get_guid().post;

            pseudo /= ss.str();
        }

        asset_->setVirtualUrl(pseudo.string());
    }

}

/**/

AssetFactory::AssetFactory(
    const non_owning_rptr<IAssetRegistry> registry_,
    const non_owning_rptr<system::InMemAssetRepository> repository_
) noexcept :
    _registry(registry_),
    _repository(repository_) {}

AssetFactory::~AssetFactory() noexcept = default;

fs::Url AssetFactory::resolveAsSource(cref<string> url_) const noexcept {

    auto cwd { _STD filesystem::current_path() };
    cwd.append(url_);

    return fs::Url { "file"sv, cwd.generic_string() };
}

void AssetFactory::prepare() {

    using namespace ::hg::engine::serialization;

    auto& layouts { LayoutManager::get() };
    sptr<DataLayoutBase> cur { nullptr };

    /* Geometry */

    /* Material */

    /* Sound */

    /* Texture */
    cur = make_sptr<DataLayout<Texture>>();
    cur->reflect().storeType<Texture>();
    cur->describe();

    layouts.storeLayout("Assets::Texture"sv, cur);
    layouts.storeLayout(Texture::typeId, cur);
    layouts.storeLayout(TypedMetaClass<Texture>::get(), cur);

    cur = make_sptr<DataLayout<Image>>();
    cur->reflect().storeType<Image>();
    cur->describe();

    layouts.storeLayout("Assets::Image"sv, cur);
    layouts.storeLayout(Image::typeId, cur);
    layouts.storeLayout(TypedMetaClass<Image>::get(), cur);
}

ptr<Font> AssetFactory::createFontAsset(cref<asset_guid> guid_) const {

    auto* instance = new Font(guid_, Vector<fs::Url> {});

    storeDefaultNameAndUrl(instance, {});
    _registry->insert({ instance });
    return instance;
}

ptr<Font> AssetFactory::createFontAsset(cref<asset_guid> guid_, cref<string> url_) const {

    auto src { resolveAsSource(url_) };
    Vector<fs::Url> sources {};

    if (src.hasScheme()/* Fast empty check */) {
        sources.push_back(src);
    }

    auto* instance = new Font(guid_, _STD move(sources));

    storeDefaultNameAndUrl(instance, url_);
    _registry->insert({ instance });
    return instance;
}

ptr<GfxMaterial> AssetFactory::createGfxMaterialAsset() const {

    auto guid = generate_asset_guid();
    return createGfxMaterialAsset(guid);
}

ptr<GfxMaterial> AssetFactory::createGfxMaterialAsset(cref<asset_guid> guid_) const {

    auto* instance = new GfxMaterial(guid_);

    storeDefaultNameAndUrl(instance, {});
    _registry->insert({ instance });
    return instance;
}

ptr<GfxMaterialPrototype> AssetFactory::createGfxMaterialPrototypeAsset(cref<asset_guid> guid_) const {

    auto* instance = new GfxMaterialPrototype(guid_);

    storeDefaultNameAndUrl(instance, {});
    _registry->insert({ instance });
    return instance;
}

ptr<Image> AssetFactory::createImageAsset() const {

    auto guid = generate_asset_guid();
    return createImageAsset(guid);
}

ptr<Image> AssetFactory::createImageAsset(cref<asset_guid> guid_) const {

    auto* instance = new Image(guid_, Vector<fs::Url> {});

    storeDefaultNameAndUrl(instance, {});
    _registry->insert({ instance });
    return instance;
}

ptr<Image> AssetFactory::createImageAsset(cref<asset_guid> guid_, cref<string> url_) const {

    auto src { resolveAsSource(url_) };
    Vector<fs::Url> sources {};

    if (src.hasScheme()/* Fast empty check */) {
        sources.push_back(src);
    }

    auto* instance = new Image(guid_, _STD move(sources));

    storeDefaultNameAndUrl(instance, string { src.path() });
    _registry->insert({ instance });
    return instance;
}

ptr<LandscapeGeometry> AssetFactory::createLandscapeGeometryAsset(cref<asset_guid> guid_) const {

    auto* instance = new LandscapeGeometry(guid_, Vector<fs::Url> {});

    storeDefaultNameAndUrl(instance, {});
    _registry->insert({ instance });
    return instance;
}

ptr<StaticGeometry> AssetFactory::createStaticGeometryAsset(cref<asset_guid> guid_) const {

    auto* instance = new StaticGeometry(
        guid_,
        Vector<fs::Url> {},
        0ui64,
        0ui64
    );

    storeDefaultNameAndUrl(instance, {});
    _registry->insert({ instance });
    return instance;
}

ptr<StaticGeometry> AssetFactory::createStaticGeometryAsset(
    cref<asset_guid> guid_,
    cref<string> url_,
    cref<u64> vertexCount_,
    cref<u64> indexCount_
) const {

    auto src { resolveAsSource(url_) };
    Vector<fs::Url> sources {};

    if (src.hasScheme()/* Fast empty check */) {
        sources.push_back(src);
    }

    auto* instance = new StaticGeometry(
        guid_,
        _STD move(sources),
        vertexCount_,
        indexCount_
    );

    storeDefaultNameAndUrl(instance, {});
    _registry->insert({ instance });
    return instance;
}

ptr<Texture> AssetFactory::createTextureAsset() const {

    auto guid = generate_asset_guid();
    return createTextureAsset(guid);
}

ptr<Texture> AssetFactory::createTextureAsset(cref<asset_guid> guid_) const {

    auto* instance = new Texture(
        guid_,
        invalid_asset_guid,
        Vector<asset_guid> { invalid_asset_guid },
        math::uivec3 {},
        gfx::TextureFormat::eUndefined,
        0,
        gfx::TextureType::eUndefined
    );

    storeDefaultNameAndUrl(instance, {});
    _registry->insert({ instance });
    return instance;
}

ptr<Texture> AssetFactory::createTextureAsset(
    cref<asset_guid> guid_,
    cref<asset_guid> baseImage_,
    mref<Vector<asset_guid>> images_,
    cref<math::uivec3> extent_,
    cref<gfx::TextureFormat> format_,
    cref<u32> mipLevel_,
    cref<gfx::TextureType> type_
) const {
    auto* instance = new Texture(
        guid_,
        baseImage_,
        _STD forward<Vector<asset_guid>>(images_),
        extent_,
        format_,
        mipLevel_,
        type_
    );

    storeDefaultNameAndUrl(instance, {});
    const auto result = _registry->insert({ instance });

    if (!result) {
        #ifdef _DEBUG
        // TODO: We need to handle a missed insertion call to the asset database
        // Important: Move from asset factory to another place, cause factory is just instantiation, and not database management
        __debugbreak();
        #else
        throw _STD runtime_error("Failed to insert texture asset into database.");
        #endif
    }

    return instance;
}
