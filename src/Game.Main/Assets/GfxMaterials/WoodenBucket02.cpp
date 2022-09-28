#include "WoodenBucket02.hpp"

#include "../Textures/DefaultAO.hpp"
#include "../Textures/WoodenBucket02Diffuse.hpp"
#include "../Textures/WoodenBucket02Normal.hpp"
#include "../Textures/WoodenBucket02Roughness.hpp"
#include "../Textures/WoodenBucket02Metalness.hpp"

using namespace ember::game::assets::material;
using namespace ember;

WoodenBucket02::WoodenBucket02() :
    GfxMaterialAsset(
        WoodenBucket02::auto_guid(),
        texture::WoodenBucket02Diffuse::auto_guid(),
        texture::DefaultAO::auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        texture::WoodenBucket02Normal::auto_guid(),
        texture::WoodenBucket02Roughness::auto_guid(),
        texture::WoodenBucket02Metalness::auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid
    ) {}
