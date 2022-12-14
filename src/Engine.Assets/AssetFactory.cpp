#include "AssetFactory.hpp"

#include <filesystem>
#include <Engine.Serialization/Layout/DataLayout.hpp>
#include <Engine.Serialization.Layouts/LayoutManager.hpp>

#include "Types/Font.hpp"
#include "Types/GfxMaterial.hpp"
#include "Types/Image.hpp"
#include "Types/LandscapeGeometry.hpp"
#include "Types/Level.hpp"
#include "Types/PfxMaterial.hpp"
#include "Types/SfxMaterial.hpp"
#include "Types/SkeletalGeometry.hpp"
#include "Types/Sound.hpp"
#include "Types/Geometry/StaticGeometry.hpp"
#include "Types/Texture/Texture.hpp"

using namespace ember::engine::assets;
using namespace ember;

AssetFactory::AssetFactory(ptr<AssetDatabase> database_) noexcept :
    _database(database_) {}

AssetFactory::~AssetFactory() noexcept = default;

Url AssetFactory::resolveAsSource(cref<string> url_) const noexcept {

    auto cwd { _STD filesystem::current_path() };
    cwd.append(url_);

    return Url { "file"sv, cwd.generic_string() };
}

void AssetFactory::prepare() {

    using namespace ::ember::engine::serialization;

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
    layouts.storeLayout(EmberClass::of<Texture>(), cur);

    cur = make_sptr<DataLayout<Image>>();
    cur->reflect().storeType<Image>();
    cur->describe();

    layouts.storeLayout("Assets::Image"sv, cur);
    layouts.storeLayout(Image::typeId, cur);
    layouts.storeLayout(EmberClass::of<Image>(), cur);
}

ptr<Font> AssetFactory::createFontAsset(cref<asset_guid> guid_) const {

    auto* instance = EmberObject::create<Font>(guid_, Vector<Url> {});

    _database->insert(guid_, Font::typeId, instance);
    return instance;
}

ptr<Font> AssetFactory::createFontAsset(cref<asset_guid> guid_, cref<string> url_) const {

    auto src { resolveAsSource(url_) };
    Vector<Url> sources {};

    if (src.hasScheme()/* Fast empty check */) {
        sources.push_back(src);
    }

    auto* instance = EmberObject::create<Font>(guid_, _STD move(sources));

    _database->insert(guid_, Font::typeId, instance);
    return instance;
}

ptr<GfxMaterial> AssetFactory::createGfxMaterialAsset() const {

    auto guid = generate_asset_guid();
    return createGfxMaterialAsset(guid);
}

ptr<GfxMaterial> AssetFactory::createGfxMaterialAsset(cref<asset_guid> guid_) const {

    auto* instance = EmberObject::create<GfxMaterial>(
        guid_,
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid
    );

    _database->insert(guid_, GfxMaterial::typeId, instance);

    return instance;
}

ptr<GfxMaterial> AssetFactory::createGfxMaterialAsset(
    cref<asset_guid> guid_,
    cref<asset_guid> albedo_,
    cref<asset_guid> ao_,
    cref<asset_guid> cavity_,
    cref<asset_guid> displacement_,
    cref<asset_guid> gloss_,
    cref<asset_guid> normal_,
    cref<asset_guid> roughness_,
    cref<asset_guid> metalness_,
    cref<asset_guid> specular_,
    cref<asset_guid> alpha_
) const {

    auto* instance = EmberObject::create<GfxMaterial>(
        guid_,
        albedo_,
        ao_,
        cavity_,
        displacement_,
        gloss_,
        normal_,
        roughness_,
        metalness_,
        specular_,
        alpha_
    );

    _database->insert(guid_, GfxMaterial::typeId, instance);
    return instance;
}

ptr<Image> AssetFactory::createImageAsset() const {

    auto guid = generate_asset_guid();
    return createImageAsset(guid);
}

ptr<Image> AssetFactory::createImageAsset(cref<asset_guid> guid_) const {

    auto* instance = EmberObject::create<Image>(guid_, Vector<Url> {});

    _database->insert(guid_, Image::typeId, instance);
    return instance;
}

ptr<Image> AssetFactory::createImageAsset(cref<asset_guid> guid_, cref<string> url_) const {

    auto src { resolveAsSource(url_) };
    Vector<Url> sources {};

    if (src.hasScheme()/* Fast empty check */) {
        sources.push_back(src);
    }

    auto* instance = EmberObject::create<Image>(guid_, _STD move(sources));

    _database->insert(guid_, Image::typeId, instance);
    return instance;
}

ptr<LandscapeGeometry> AssetFactory::createLandscapeGeometryAsset(cref<asset_guid> guid_) const {

    auto* instance = EmberObject::create<LandscapeGeometry>(guid_, Vector<Url> {});

    _database->insert(guid_, LandscapeGeometry::typeId, instance);
    return instance;
}

ptr<StaticGeometry> AssetFactory::createStaticGeometryAsset(cref<asset_guid> guid_) const {

    auto* instance = EmberObject::create<StaticGeometry>(
        guid_,
        Vector<Url> {},
        0ui64,
        0ui64
    );

    _database->insert(guid_, StaticGeometry::typeId, instance);
    return instance;
}

ptr<StaticGeometry> AssetFactory::createStaticGeometryAsset(
    cref<asset_guid> guid_,
    cref<string> url_,
    cref<u64> vertexCount_,
    cref<u64> indexCount_
) const {

    auto src { resolveAsSource(url_) };
    Vector<Url> sources {};

    if (src.hasScheme()/* Fast empty check */) {
        sources.push_back(src);
    }

    auto* instance = EmberObject::create<StaticGeometry>(
        guid_,
        _STD move(sources),
        vertexCount_,
        indexCount_
    );

    _database->insert(guid_, StaticGeometry::typeId, instance);
    return instance;
}

ptr<Texture> AssetFactory::createTextureAsset() const {

    auto guid = generate_asset_guid();
    return createTextureAsset(guid);
}

ptr<Texture> AssetFactory::createTextureAsset(cref<asset_guid> guid_) const {

    auto* instance = EmberObject::create<Texture>(
        guid_,
        invalid_asset_guid,
        Vector<asset_guid> { invalid_asset_guid },
        math::uivec3 {},
        gfx::TextureFormat::eUndefined,
        0,
        gfx::TextureType::eUndefined
    );

    _database->insert(guid_, Texture::typeId, instance);
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
    auto* instance = EmberObject::create<Texture>(
        guid_,
        baseImage_,
        _STD forward<Vector<asset_guid>>(images_),
        extent_,
        format_,
        mipLevel_,
        type_
    );

    const auto result {
        _database->insert(guid_, Texture::typeId, instance)
    };

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
