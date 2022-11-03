#include "WoodenPier01Poles.hpp"

#include "../Textures/DefaultRoughness.hpp"
#include "../Textures/DefaultAO.hpp"
#include "../Textures/DefaultNormal.hpp"
#include "../Textures/DefaultDiffuse.hpp"
#include "../Textures/WoodenPier01PolesDiffuse.hpp"
#include "../Textures/WoodenPier01PolesNormal.hpp"
#include "../Textures/DefaultMetalness.hpp"
#include "../Textures/WoodenPier01PolesRoughness.hpp"

using namespace ember::game::assets::material;
using namespace ember;

WoodenPier01Poles::WoodenPier01Poles() :
    GfxMaterialAsset(
        WoodenPier01Poles::unstable_auto_guid(),
        texture::WoodenPier01PolesDiffuse::unstable_auto_guid(),
        texture::DefaultAO::unstable_auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        texture::WoodenPier01PolesNormal::unstable_auto_guid(),
        texture::WoodenPier01PolesRoughness::unstable_auto_guid(),
        texture::DefaultMetalness::unstable_auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid
    ) {}
