#include "WoodenBucket01.hpp"

#include "../Textures/DefaultAO.hpp"
#include "../Textures/WoodenBucket01Diffuse.hpp"
#include "../Textures/WoodenBucket01Normal.hpp"
#include "../Textures/WoodenBucket01Roughness.hpp"
#include "../Textures/WoodenBucket01Metalness.hpp"

using namespace hg::game::assets::material;
using namespace hg;

WoodenBucket01::WoodenBucket01() :
    GfxMaterialAsset(
        WoodenBucket01::unstable_auto_guid(),
        texture::WoodenBucket01Diffuse::unstable_auto_guid(),
        texture::DefaultAO::unstable_auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        texture::WoodenBucket01Normal::unstable_auto_guid(),
        texture::WoodenBucket01Roughness::unstable_auto_guid(),
        texture::WoodenBucket01Metalness::unstable_auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid
    ) {}
