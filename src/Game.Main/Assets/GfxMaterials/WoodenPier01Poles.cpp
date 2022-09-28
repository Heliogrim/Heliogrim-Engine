#include "WoodenPier01Poles.hpp"

#include "../Textures/DefaultRoughness.hpp"
#include "../Textures/DefaultAO.hpp"
#include "../Textures/DefaultNormal.hpp"
#include "../Textures/DefaultDiffuse.hpp"
#include "../Textures/WoodenPier01PolesDiffuse.hpp"

using namespace ember::game::assets::material;
using namespace ember;

WoodenPier01Poles::WoodenPier01Poles() :
    GfxMaterialAsset(
        WoodenPier01Poles::auto_guid(),
        texture::WoodenPier01PolesDiffuse::auto_guid(),
        texture::DefaultAO::auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        texture::DefaultNormal::auto_guid(),
        texture::DefaultRoughness::auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid
    ) {}
