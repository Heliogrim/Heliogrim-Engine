#include "WoodenPier01Planks.hpp"

#include "../Textures/DefaultRoughness.hpp"
#include "../Textures/DefaultAO.hpp"
#include "../Textures/DefaultNormal.hpp"
#include "../Textures/DefaultDiffuse.hpp"
#include "../Textures/WoodenPier01PlanksDiffuse.hpp"

using namespace ember::game::assets::material;
using namespace ember;

WoodenPier01Planks::WoodenPier01Planks() :
    GfxMaterialAsset(
        WoodenPier01Planks::auto_guid(),
        texture::WoodenPier01PlanksDiffuse::auto_guid(),
        texture::DefaultAO::auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        texture::DefaultNormal::auto_guid(),
        texture::DefaultRoughness::auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid
    ) {}
