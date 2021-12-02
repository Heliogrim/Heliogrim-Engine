#include "AssetFactory.hpp"

#include "Types/GfxMaterial.hpp"
#include "Types/Image.hpp"
#include "Types/LandscapeGeometry.hpp"
#include "Types/Level.hpp"
#include "Types/PfxMaterial.hpp"
#include "Types/SfxMaterial.hpp"
#include "Types/SkeletalGeometry.hpp"
#include "Types/StaticGeometry.hpp"
#include "Types/Sound.hpp"
#include "Types/Texture.hpp"

using namespace ember::engine::assets;
using namespace ember;

AssetFactory::AssetFactory(ptr<AssetDatabase> database_) noexcept :
    _database(database_) {}

AssetFactory::~AssetFactory() noexcept = default;

ptr<engine::assets::Asset> AssetFactory::createGfxMaterialAsset() {

    auto guid = generate_asset_guid();
    return createGfxMaterialAsset(guid);
}

ptr<engine::assets::Asset> AssetFactory::createGfxMaterialAsset(cref<asset_guid> guid_) {

    auto* instance = new GfxMaterial {
        guid_,
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid
    };

    _database->insert(guid_, GfxMaterial::type_id, instance);

    return instance;
}

ptr<engine::assets::Asset> AssetFactory::createGfxMaterialAsset(
    cref<asset_guid> guid_,
    cref<asset_guid> albedo_,
    cref<asset_guid> ao_,
    cref<asset_guid> cavity_,
    cref<asset_guid> displacement_,
    cref<asset_guid> gloss_,
    cref<asset_guid> normal_,
    cref<asset_guid> roughness_,
    cref<asset_guid> specular_
) {

    auto* instance = new GfxMaterial {
        guid_,
        albedo_,
        ao_,
        cavity_,
        displacement_,
        gloss_,
        normal_,
        roughness_,
        specular_
    };

    _database->insert(guid_, GfxMaterial::type_id, instance);

    return instance;
}

ptr<engine::assets::Asset> AssetFactory::createImageAsset() {

    auto guid = generate_asset_guid();
    return createImageAsset(guid);
}

ptr<engine::assets::Asset> AssetFactory::createImageAsset(cref<asset_guid> guid_) {

    auto* instance = new Image {
        guid_,
        ""
    };

    _database->insert(guid_, Image::type_id, instance);

    return instance;
}

ptr<engine::assets::Asset> AssetFactory::createImageAsset(cref<asset_guid> guid_, cref<string> url_) {

    auto* instance = new Image {
        guid_,
        url_
    };

    _database->insert(guid_, Image::type_id, instance);

    return instance;
}

ptr<engine::assets::Asset> AssetFactory::createTextureAsset() {

    auto guid = generate_asset_guid();
    return createTextureAsset(guid);
}

ptr<engine::assets::Asset> AssetFactory::createTextureAsset(cref<asset_guid> guid_) {

    auto* instance = new Texture {
        guid_,
        invalid_asset_guid,
        { invalid_asset_guid },
        {},
        gfx::TextureFormat::eUndefined,
        0,
        gfx::TextureType::eUndefined
    };

    _database->insert(guid_, Texture::type_id, instance);

    return instance;
}

ptr<engine::assets::Asset> AssetFactory::createTextureAsset(
    cref<asset_guid> guid_,
    cref<asset_guid> baseImage_,
    mref<Vector<asset_guid>> images_,
    cref<math::uivec3> extent_,
    cref<gfx::TextureFormat> format_,
    cref<u32> mipLevel_,
    cref<gfx::TextureType> type_
) {
    auto* instance = new Texture {
        guid_,
        baseImage_,
        _STD forward<Vector<asset_guid>>(images_),
        extent_,
        format_,
        mipLevel_,
        type_
    };

    _database->insert(guid_, Texture::type_id, instance);

    return instance;
}
