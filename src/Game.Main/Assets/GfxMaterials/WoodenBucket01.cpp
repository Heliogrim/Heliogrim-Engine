#include "WoodenBucket01.hpp"

#include "../Textures/DefaultAO.hpp"
#include "../Textures/WoodenBucket01Diffuse.hpp"
#include "../Textures/WoodenBucket01Normal.hpp"
#include "../Textures/WoodenBucket01Roughness.hpp"

using namespace ember::game::assets::material;
using namespace ember;

WoodenBucket01::WoodenBucket01() :
    GfxMaterialAsset(
        WoodenBucket01::auto_guid(),
        texture::WoodenBucket01Diffuse::auto_guid(),
        texture::DefaultAO::auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        texture::WoodenBucket01Normal::auto_guid(),
        texture::WoodenBucket01Roughness::auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid
    ) {}
